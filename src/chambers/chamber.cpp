#include "chamber.hpp"
#include "../global-settings/setting-manager.hpp"
#include <queue>
#include <cmath>
#include <algorithm>
#include "../utils/collision-solver.hpp"
#include "../entities/enemy/sprinter.hpp"
#include "../entities/enemy/soldier.hpp"
#include "../entities/enemy/brute.hpp"
#include "../entities/effects/slowed-effect.hpp"
#include "../entities/player.hpp"
#include "../utils/math-utility.hpp"
#include "../graphics/particle-system.hpp"
#include "../ui/widgets/floating-text-manager.hpp"
#include "../global-settings/sound-manager.hpp"
#include "tilemap-loader.hpp"

Chamber::Chamber(Player& player) : player(player), isCompleted(false) {
    player.setChamber(this);
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
            (*it)->onDeath(this);
            if (dropsFragments) {
                itemManager.spawnEnemyDrops((*it).get(), player, *this);
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
    
    drawForeground(window);
    
    for (const auto& hb : debugHitboxes) {
        CollisionSolver::drawDebug(window, hb.shape);
    }
    debugHitboxes.clear();
}

void Chamber::spawnArrow(sf::Vector2f startPos, sf::Vector2f direction, float maxDistance, float speed, ArrowHitMode hitMode) {
    activeArrows.emplace_back(startPos, direction, maxDistance, speed, hitMode);
}


void Chamber::setGrids2D5(const std::vector<std::vector<std::string>>& newTypeGrid, const std::vector<std::vector<int>>& newLevelGrid) {
    typeGrid = newTypeGrid;
    levelGrid = newLevelGrid;
    
    float cellSize = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();

    // Synthesize the full 2.5D render data (includes the walkableGrid bitmask).
    // Store it so getWalkableGrid() returns the correct graph for the pathfinder.
    // Previously createRenderableMap() built and then silently discarded this data,
    // leaving renderData2D5.walkableGrid permanently empty and forcing the pathfinder
    // to fall back to the flat typeGrid BFS that ignores elevation entirely.
    renderData2D5 = TilemapLoader::getInstance().synthesizeMap(typeGrid, levelGrid);

    tileMap = TilemapLoader::getInstance().createRenderableMap(typeGrid, levelGrid, cellSize, ox, oy);
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

    int charLevel = 0; // Default Level 0
    bool isOnStairs = false;
    if (!levelGrid.empty() && ty >= 0 && ty < levelGrid.size() && tx >= 0 && tx < levelGrid[0].size()) {
        if (typeGrid[ty][tx] == "S") {
            isOnStairs = true;
        } else if (typeGrid[ty][tx] == "V") {
            if (ty > 0 && typeGrid[ty-1][tx] != "0" && typeGrid[ty-1][tx] != "W") charLevel = levelGrid[ty-1][tx] - 1;
            else if (ty < levelGrid.size() - 1 && typeGrid[ty+1][tx] != "0" && typeGrid[ty+1][tx] != "W") charLevel = levelGrid[ty+1][tx] - 1;
        } else if (typeGrid[ty][tx] == "H") {
            if (tx > 0 && typeGrid[ty][tx-1] != "0" && typeGrid[ty][tx-1] != "W") charLevel = levelGrid[ty][tx-1] - 1;
            else if (tx < levelGrid[0].size() - 1 && typeGrid[ty][tx+1] != "0" && typeGrid[ty][tx+1] != "W") charLevel = levelGrid[ty][tx+1] - 1;
        } else {
            charLevel = levelGrid[ty][tx] - 1;
        }
        if (charLevel < 0) charLevel = 0;
    }

    if (isOnStairs) {
        // On stairs: transitioning between height levels, neither level ground blocks movement
    } else if (charLevel == 0) {
        // On ground level: elevated tiles and cliff faces are solid walls.
        obs.insert(obs.end(), elevationObstacles.begin(), elevationObstacles.end());
    } else {
        // On level 1: ground tiles are solid walls, preventing walking off the edge.
        obs.insert(obs.end(), inverseElevationObstacles.begin(), inverseElevationObstacles.end());
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
    if (level < 0) level = 0;
    return level;
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

    // Apply Wraithblade knockback if active form is Wraithblade
    if (!lethal && player.getActiveFormType() == FormType::WRAITHBLADE) {
        if (enemy->canBeKnockedBack()) {
            sf::Vector2f dir = Math::normalize(enemy->getPosition() - player.getPosition());
            enemy->applyKnockback(dir, 1600.0f);
        }
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
        for (size_t y = 0; y < rows; ++y) {
            for (size_t x = 0; x < cols; ++x) {
                sf::FloatRect rect({ox + x * size, oy + y * size}, {size, size});
                std::string type = typeGrid[y][x];
                int level = levelGrid[y][x];

                // Water and Void are base obstacles
                if (type == "W" || type == "0") {
                    baseObstacles.push_back(rect);
                } else if (type != "S") { // Stairs are ramps connecting levels
                    // Level 2+ is an obstacle for Level 0 chars (level 1 in grid).
                    if (level > 1) {
                        elevationObstacles.push_back(rect);
                    }
                    // A cell below a higher elevation level (y > 0 && levelGrid[y-1][x] > level) is a cliff face!
                    // Ground-level characters cannot walk onto cliff faces; only stairs ("S") allow elevation changes.
                    if (y > 0 && levelGrid[y - 1][x] > level) {
                        elevationObstacles.push_back(rect);
                    }
                    // Level 1 is a drop (obstacle) for Level 1 chars (level 2 in grid).
                    if (level == 1) {
                        inverseElevationObstacles.push_back(rect);
                    }
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
    std::cout << "[Chamber] All enemies frozen for " << duration << " seconds!\n";
}

int Chamber::processPlayerAttack(const Hitbox& hitbox) {
    int killsThisAttack = 0;
    int totalHits = 0;
    std::vector<Enemy*> killedEnemies;

    for (auto& enemy : enemies) {
        if (!enemy->isAlive()) continue;

        if (CollisionSolver::checkCollision(hitbox, enemy->getBounds())) {
            totalHits++;
            float damage = player.getEffectiveStats().damage;
            if (player.getStateMachine().getActiveState()) {
                damage = player.getStateMachine().getActiveState()->modifyOutgoingDamage(damage);
            }
            
            enemy->takeDamage(damage);
            
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
            std::cout << "Voidcaster pierce-kill! +1 Bonus Fragment queued.\n";
        }
    }



    return totalHits;
}