#include "chamber.hpp"
#include "../global-settings/setting-manager.hpp"
#include <queue>
#include <cmath>
#include <algorithm>
#include "../utils/collision-solver.hpp"
#include "../utils/grid-passability.hpp"
#include "../entities/enemy/sprinter.hpp"
#include "../entities/enemy/soldier.hpp"
#include "../entities/enemy/brute.hpp"
#include "../entities/effects/slowed-effect.hpp"
#include "../entities/player.hpp"
#include "../utils/math-utility.hpp"
#include "../ui/graphics/particle-system.hpp"
#include "../ui/graphics/aura-renderer.hpp"
#include "../utils/camera.hpp"
#include "../ui/widgets/floating-text-manager.hpp"
#include "../global-settings/sound-manager.hpp"
#include "tilemap-loader.hpp"

Chamber::Chamber(Player& player) : player(player), isCompleted(false) {
    player.setChamber(this);
    exitGate = std::make_unique<ExitGate>();
}

void Chamber::setWaves(const std::vector<WaveConfig>& configs) {
    waveSpawner.setWaves(configs);
}

void Chamber::update(float dt) {
    if (!player.isAlive()) {
        if (observer) {
            observer->onChamberFailed();
        }
        return;
    }

    waveSpawner.update(dt, *this, player);

    bool wasFrozen = (freezeTimer > 0.0f);
    if (wasFrozen) {
        freezeTimer -= dt;
        if (freezeTimer <= 0.0f) {
            for (auto& enemy : enemies) {
                if (enemy && enemy->isAlive()) {
                    enemy->setFrozen(false);
                    enemy->setTint(enemy->getStatusEffects().empty() ? sf::Color::White : enemy->getStatusEffects().back()->getColor());
                }
            }
        }
    }
    bool frozen = (freezeTimer > 0.0f);

    for (auto it = enemies.begin(); it != enemies.end(); ) {
        if (!(*it)->isAlive()) {
            if (!(*it)->getHasEscaped()) {
                (*it)->onDeath(this);
                if (dropsFragments) {
                    itemManager.spawnEnemyDrops((*it).get(), player, *this);
                }
            }
            it = enemies.erase(it);
        } else {
            if (frozen) {
                (*it)->setVelocity({0.0f, 0.0f});
                (*it)->update(dt);
            } else {
                (*it)->update(dt);
                if ((*it)->isAlive()) {
                    (*it)->updateState(dt, *this);
                }
            }
            ++it;
        }
    }
    
    if (!pendingEnemies.empty()) {
        for (auto& pending : pendingEnemies) {
            enemies.push_back(std::move(pending));
        }
        pendingEnemies.clear();
    }
    
    itemManager.update(dt, player, *this);

    for (auto it = activeArrows.begin(); it != activeArrows.end(); ) {
        it->update(dt, *this, player);
        if (!it->isActive()) {
            it = activeArrows.erase(it);
        } else {
            ++it;
        }
    }

    checkCollisions(dt);

    if (exitGate) {
        exitGate->update(dt);
    }
}

void Chamber::draw(sf::RenderWindow& window) {
    window.draw(tileMap);
    
    drawBackground(window);
    
    itemManager.draw(window);
    for (const auto& arrow : activeArrows) {
        arrow.draw(window);
    }
    for (const auto& enemy : enemies) {
        enemy->draw(window);
    }
    
    if (exitGate && exitGate->isActive()) {
        exitGate->draw(window);
    }

    drawForeground(window);
    
    for (const auto& hb : debugHitboxes) {
        CollisionSolver::drawDebug(window, hb.shape);
    }
    debugHitboxes.clear();
}

void Chamber::spawnArrow(sf::Vector2f startPos, sf::Vector2f direction, float maxDistance, float speed, ArrowHitMode hitMode, bool isRedLaser) {
    activeArrows.emplace_back(startPos, direction, maxDistance, speed, hitMode, isRedLaser);
}

void Chamber::setExitPosition(const sf::Vector2f& pos) {
    exitPosition = pos;
    if (exitGate) {
        exitGate->setPosition(pos);
    }
}

sf::Vector2f Chamber::getExitPosition() const {
    return exitPosition;
}


void Chamber::setGrids2D5(const std::vector<std::vector<std::string>>& newTypeGrid, const std::vector<std::vector<int>>& newLevelGrid, const std::vector<std::vector<std::string>>& newBridgeGrid) {
    typeGrid = newTypeGrid;
    levelGrid = newLevelGrid;
    bridgeGrid = newBridgeGrid;
    
    float cellSize = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();

    // Synthesize the full 2.5D render data (includes the walkableGrid bitmask).
    // Store it so getWalkableGrid() returns the correct graph for the pathfinder.
    renderData2D5 = TilemapLoader::getInstance().synthesizeMap(typeGrid, levelGrid, bridgeGrid);

    tileMap = TilemapLoader::getInstance().createRenderableMap(typeGrid, levelGrid, cellSize, ox, oy, bridgeGrid);
    buildObstaclesFromGrid();
}

std::vector<sf::FloatRect> Chamber::getObstaclesFor(const Character* character) const {
    std::vector<sf::FloatRect> obs = baseObstacles;
    
    if (!character || typeGrid.empty() || typeGrid[0].empty()) return obs;

    sf::FloatRect bounds = character->getBounds();
    sf::Vector2f trueCenter = {bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f}; // True center

    float cellSize = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();

    int tx = static_cast<int>((trueCenter.x - ox) / cellSize);
    int ty = static_cast<int>((trueCenter.y - oy) / cellSize);

    auto getBridgeAt = [&](int r, int c) -> std::string {
        if (bridgeGrid.empty() || r < 0 || r >= static_cast<int>(bridgeGrid.size()) || c < 0 || c >= static_cast<int>(bridgeGrid[r].size())) return ".";
        const std::string& b = bridgeGrid[r][c];
        return b.empty() ? "." : b;
    };

    if (typeGrid.empty() || levelGrid.empty() || ty < 0 || ty >= static_cast<int>(levelGrid.size()) || tx < 0 || tx >= static_cast<int>(levelGrid[0].size())) {
        return obs;
    }

    bool isOnStairs = (typeGrid[ty][tx] == "S");
    int charLevel = levelGrid[ty][tx];

    if (!isOnStairs) {
        GridData gridData{typeGrid, levelGrid, bridgeGrid};
        int rows = static_cast<int>(typeGrid.size());
        int cols = static_cast<int>(typeGrid[0].size());

        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
                if (y == ty && x == tx) continue;

                std::string t = typeGrid[y][x];
                std::string br = getBridgeAt(y, x);
                bool isBridge = (br == "V" || br == "H");
                int cellLvl = levelGrid[y][x];

                // If cell (y, x) elevation differs from character's elevation and is not Stairs ("S") or Bridge,
                // it is a solid elevation boundary obstacle for the character.
                if (cellLvl != charLevel && t != "S" && !isBridge) {
                    sf::FloatRect rect({ox + x * cellSize, oy + y * cellSize}, {cellSize, cellSize});
                    obs.push_back(rect);
                } else if (GridPassability::isCliffFace(y, x, levelGrid, typeGrid) && t != "S" && !isBridge) {
                    // Cliff face tiles below higher platforms are solid for lower level characters
                    if (charLevel <= levelGrid[y - 1][x] - 1) {
                        sf::FloatRect rect({ox + x * cellSize, oy + y * cellSize}, {cellSize, cellSize});
                        obs.push_back(rect);
                    }
                }
            }
        }
    }

    return obs;
}

int Chamber::getElevationLevelAt(sf::Vector2f pos) const {
    if (levelGrid.empty() || typeGrid.empty() || typeGrid[0].empty()) return 0;

    float cellSize = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();

    int tx = static_cast<int>((pos.x - ox) / cellSize);
    int ty = static_cast<int>((pos.y - oy) / cellSize);

    if (ty < 0 || ty >= static_cast<int>(levelGrid.size()) || tx < 0 || tx >= static_cast<int>(levelGrid[0].size())) {
        return 0;
    }

    int level = levelGrid[ty][tx] - 1;
    if (level < 0) {
        if (!bridgeGrid.empty() && ty < static_cast<int>(bridgeGrid.size()) && tx < static_cast<int>(bridgeGrid[ty].size())) {
            std::string br = bridgeGrid[ty][tx];
            if (br == "V") {
                if (ty > 0 && levelGrid[ty-1][tx] > 0) level = levelGrid[ty-1][tx] - 1;
                else if (ty < static_cast<int>(levelGrid.size()) - 1 && levelGrid[ty+1][tx] > 0) level = levelGrid[ty+1][tx] - 1;
            } else if (br == "H") {
                if (tx > 0 && levelGrid[ty][tx-1] > 0) level = levelGrid[ty][tx-1] - 1;
                else if (tx < static_cast<int>(levelGrid[0].size()) - 1 && levelGrid[ty][tx+1] > 0) level = levelGrid[ty][tx+1] - 1;
            }
        }
    }
    if (level < 0) level = 0;
    return level;
}

bool Chamber::isStairsAt(sf::Vector2f pos) const {
    if (typeGrid.empty() || typeGrid[0].empty()) return false;

    float cellSize = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();

    int tx = static_cast<int>((pos.x - ox) / cellSize);
    int ty = static_cast<int>((pos.y - oy) / cellSize);

    if (ty < 0 || ty >= static_cast<int>(typeGrid.size()) || tx < 0 || tx >= static_cast<int>(typeGrid[0].size())) {
        return false;
    }

    return (typeGrid[ty][tx] == "S");
}

bool Chamber::isOnStairs(const Character* character) const {
    if (!character) return false;
    sf::FloatRect bounds = character->getBounds();
    sf::Vector2f trueCenter = {bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f};
    return isStairsAt(trueCenter);
}

std::vector<sf::FloatRect> Chamber::getArrowSolidObstacles(sf::Vector2f shooterPos) const {
    std::vector<sf::FloatRect> obs;
    if (typeGrid.empty() || typeGrid[0].empty()) return obs;

    float size = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();
    int rows = typeGrid.size();
    int cols = typeGrid[0].size();

    float mapWidth = cols * size;
    float mapHeight = rows * size;
    float thickness = 1000.f;

    obs.push_back(sf::FloatRect({ox - thickness, oy - thickness}, {mapWidth + 2 * thickness, thickness})); // Top
    obs.push_back(sf::FloatRect({ox - thickness, oy + mapHeight}, {mapWidth + 2 * thickness, thickness})); // Bottom
    obs.push_back(sf::FloatRect({ox - thickness, oy}, {thickness, mapHeight})); // Left
    obs.push_back(sf::FloatRect({ox + mapWidth, oy}, {thickness, mapHeight})); // Right
    obs.push_back(sf::FloatRect({ox, oy}, {mapWidth, size})); // Top wall-front row 0

    int shooterLevel = getElevationLevelAt(shooterPos);

    // Dynamic elevation comparison:
    // Any land tile with elevation level higher than the shooter's elevation (tileLevel > shooterLevel)
    // acts as a solid height obstacle blocking the arrow.
    if (!levelGrid.empty()) {
        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
                int tileLevel = levelGrid[y][x] - 1;
                if (tileLevel < 0) tileLevel = 0;

                std::string type = typeGrid[y][x];
                // Water and Void chasms are not land height obstacles for flying arrows
                if (tileLevel > shooterLevel && type != "W" && type != "0") {
                    sf::FloatRect rect({ox + x * size, oy + y * size}, {size, size});
                    obs.push_back(rect);
                }
            }
        }
    }

    return obs;
}

void Chamber::onFragmentCollected(float value) {
    // Default implementation does nothing
}

void Chamber::onEnemyHit(Enemy* enemy, bool lethal) {
    if (player.getStateMachine().getActiveState()) {
        player.getStateMachine().getActiveState()->onEnemyHit(player, enemy, lethal, *this);
    }
}

void Chamber::completeChamber() {
    if (isCompleted || isFailed) return;
    isCompleted = true;
    if (observer) {
        observer->onChamberCompleted();
    }
}

void Chamber::failChamber() {
    if (isCompleted || isFailed) return;
    isFailed = true;
    if (observer) {
        observer->onChamberFailed();
    }
}

std::vector<Enemy*> Chamber::getEnemiesRaw() const {
    std::vector<Enemy*> raw;
    for (const auto& e : enemies) {
        raw.push_back(e.get());
    }
    return raw;
}


void Chamber::buildObstaclesFromGrid() {
    baseObstacles.clear();
    elevationObstacles.clear();
    inverseElevationObstacles.clear();

    if (typeGrid.empty() || typeGrid[0].empty()) return;

    float size = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();

    int rows = typeGrid.size();
    int cols = typeGrid[0].size();
    
    // Implicit Map Boundaries
    float mapWidth = cols * size;
    float mapHeight = rows * size;
    float thickness = 1000.f; // arbitrary thick wall to prevent escaping
    
    baseObstacles.push_back(sf::FloatRect({ox - thickness, oy - thickness}, {mapWidth + 2 * thickness, thickness})); // Top
    baseObstacles.push_back(sf::FloatRect({ox - thickness, oy + mapHeight}, {mapWidth + 2 * thickness, thickness})); // Bottom
    baseObstacles.push_back(sf::FloatRect({ox - thickness, oy}, {thickness, mapHeight})); // Left
    baseObstacles.push_back(sf::FloatRect({ox + mapWidth, oy}, {thickness, mapHeight})); // Right

    // Row 0 is always rendered as a wall-front decorative strip (see TilemapLoader).
    // Block movement into it so characters never walk behind that visual wall.
    baseObstacles.push_back(sf::FloatRect({ox, oy}, {mapWidth, size})); // Top wall-front row

    if (!typeGrid.empty() && !levelGrid.empty()) {
        auto getBridgeAt = [&](int r, int c) -> std::string {
            if (bridgeGrid.empty() || r < 0 || r >= rows || c < 0 || c >= cols) return ".";
            if (r >= static_cast<int>(bridgeGrid.size()) || c >= static_cast<int>(bridgeGrid[r].size())) return ".";
            const std::string& b = bridgeGrid[r][c];
            return b.empty() ? "." : b;
        };

        auto isWalkableTerrain = [&](int r, int c) -> bool {
            if (r < 0 || r >= rows || c < 0 || c >= cols) return false;
            std::string br = getBridgeAt(r, c);
            if (br == "V" || br == "H") return true;
            std::string t = typeGrid[r][c];
            return (t == "L" || t == "S" || t == "E" || t == "X");
        };

        for (size_t y = 0; y < rows; ++y) {
            for (size_t x = 0; x < cols; ++x) {
                sf::FloatRect rect({ox + x * size, oy + y * size}, {size, size});
                std::string type = typeGrid[y][x];
                int level = levelGrid[y][x];
                std::string bridge = getBridgeAt(static_cast<int>(y), static_cast<int>(x));
                bool hasBridge = (bridge == "V" || bridge == "H");

                if (hasBridge) {
                    float sideThick = 2.0f;
                    int iy = static_cast<int>(y);
                    int ix = static_cast<int>(x);
                    if (bridge == "V") {
                        // Vertical bridge: travel is North-South.
                        // Left guardrail if left cell is impassable
                        if (!isWalkableTerrain(iy, ix - 1)) {
                            baseObstacles.push_back(sf::FloatRect({ox + x * size, oy + y * size}, {sideThick, size}));
                        }
                        // Right guardrail if right cell is impassable
                        if (!isWalkableTerrain(iy, ix + 1)) {
                            baseObstacles.push_back(sf::FloatRect({ox + x * size + size - sideThick, oy + y * size}, {sideThick, size}));
                        }
                    } else if (bridge == "H") {
                        // Horizontal bridge: travel is East-West.
                        // Top guardrail if top cell is impassable
                        if (!isWalkableTerrain(iy - 1, ix)) {
                            baseObstacles.push_back(sf::FloatRect({ox + x * size, oy + y * size}, {size, sideThick}));
                        }
                        // Bottom guardrail if bottom cell is impassable
                        if (!isWalkableTerrain(iy + 1, ix)) {
                            baseObstacles.push_back(sf::FloatRect({ox + x * size, oy + y * size + size - sideThick}, {size, sideThick}));
                        }
                    }
                } else if (type == "W" || type == "0") {
                    baseObstacles.push_back(rect);
                } else if (type == "S") {
                    // Prevent players from walking off the side of the stairs
                    bool isVertical = true; // Assume vertical stairs
                    if (x > 0 && x < cols - 1 && levelGrid[y][x-1] != levelGrid[y][x+1]) {
                        isVertical = false;
                    }
                    
                    float sideThick = 2.0f; // Thin wall
                    if (isVertical) {
                        // Left wall (only if the tile to the left is not also stairs)
                        if (x == 0 || typeGrid[y][x-1] != "S") {
                            baseObstacles.push_back(sf::FloatRect({ox + x * size, oy + y * size}, {sideThick, size}));
                        }
                        // Right wall
                        if (x == cols - 1 || typeGrid[y][x+1] != "S") {
                            baseObstacles.push_back(sf::FloatRect({ox + x * size + size - sideThick, oy + y * size}, {sideThick, size}));
                        }
                    } else {
                        // Top wall
                        if (y == 0 || typeGrid[y-1][x] != "S") {
                            baseObstacles.push_back(sf::FloatRect({ox + x * size, oy + y * size}, {size, sideThick}));
                        }
                        // Bottom wall
                        if (y == rows - 1 || typeGrid[y+1][x] != "S") {
                            baseObstacles.push_back(sf::FloatRect({ox + x * size, oy + y * size + size - sideThick}, {size, sideThick}));
                        }
                    }
                } else { // Land ("L", "E", "X")
                    if (level > 1) {
                        elevationObstacles.push_back(rect);
                    }
                    // A cell below a higher elevation level is a cliff face!
                    if (y > 0 && levelGrid[y - 1][x] > level && typeGrid[y - 1][x] != "S") {
                        elevationObstacles.push_back(rect);
                    }
                    if (level == 1) {
                        inverseElevationObstacles.push_back(rect);
                    }
                }
            }
        }
    }
}

void Chamber::spawnEnemy(std::unique_ptr<Enemy> enemy) {
    pendingEnemies.push_back(std::move(enemy));
}

void Chamber::checkCollisions(float dt) {
    // Resolve enemy collisions with obstacles.
    // Split into X and Y passes with obstacle refresh between them
    // so that if X-axis movement changes the character's elevation level,
    // the Y-axis pass uses the correct obstacle set.
    for (auto& enemy : enemies) {
        CollisionSolver::resolveX(*enemy, getObstaclesFor(enemy.get()), dt);
        CollisionSolver::resolveY(*enemy, getObstaclesFor(enemy.get()), dt);
    }
}

void Chamber::freezeAllEnemies(float duration) {
    freezeTimer = duration;
    for (const auto& enemy : enemies) {
        if (enemy && enemy->isAlive()) {
            enemy->setFrozen(true);
            enemy->setTint(sf::Color(100, 220, 255));
            sf::Vector2f headPos = enemy->getPosition() + sf::Vector2f(0.0f, -35.0f);
            UI::FloatingTextManager::getInstance().spawnStatus(headPos, "FROZEN", sf::Color(100, 220, 255));
            ParticleSystem::getInstance().emitBurst(enemy->getPosition(), 15, sf::Color(100, 220, 255, 200), 20.0f, 60.0f, 0.5f, 1.0f, 3.0f);
        }
    }
}

int Chamber::processPlayerAttack(const Hitbox& hitbox) {
    int killsThisAttack = 0;
    int totalHits = 0;
    std::vector<Enemy*> killedEnemies;

    float critRate = player.getCriticalHitRate();

    for (auto& enemy : enemies) {
        if (!enemy->isAlive()) continue;

        if (CollisionSolver::checkCollision(hitbox, enemy->getBounds())) {
            totalHits++;
            float baseDamage = player.getEffectiveStats().damage;
            if (player.getStateMachine().getActiveState()) {
                baseDamage = player.getStateMachine().getActiveState()->modifyOutgoingDamage(baseDamage);
            }

            // Check Critical Hit (only consider if enemy is not shielded)
            bool isCrit = false;
            if (!enemy->isShielded() && critRate > 0.0f) {
                float roll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                if (roll < critRate) {
                    isCrit = true;
                }
            }

            float finalDamage = isCrit ? (baseDamage * 2.0f) : baseDamage;
            enemy->takeDamage(finalDamage, isCrit);

            if (isCrit) {
                // Visual feedback: Screen White Flash, Camera Shake, and Critical Particle Burst
                AuraRenderer::getInstance().triggerScreenFlash(sf::Color(255, 255, 255, 210), 0.14f);
                Camera::triggerShake(7.5f, 0.18f);
                ParticleSystem::getInstance().emitBurst(enemy->getPosition(), 25, sf::Color(255, 60, 60, 240), 60.0f, 180.0f, 0.2f, 0.6f, 4.5f);

                // Push enemy backward only on critical hit
                if (enemy->canBeKnockedBack()) {
                    sf::Vector2f dir = Math::normalize(enemy->getPosition() - player.getPosition());
                    enemy->applyKnockback(dir, 1600.0f);
                }
            }

            bool lethal = !enemy->isAlive();
            if (lethal) {
                killsThisAttack++;
                killedEnemies.push_back(enemy.get());
            }

            onEnemyHit(enemy.get(), lethal);
        }
    }

    // Voidcaster Multiplier: +1 fragment per additional enemy killed beyond the first in one shot
    if (player.getActiveFormType() == FormType::VOIDCASTER && killsThisAttack > 1) {
        for (size_t i = 1; i < killedEnemies.size(); ++i) {
            killedEnemies[i]->addBonusFragments(1);
        }
    }

    return totalHits;
}