#include "chamber.hpp"
#include "../global-settings/setting-manager.hpp"
#include <queue>
#include <cmath>
#include <algorithm>
#include "../utils/collision-solver.hpp"
#include "../entities/enemy/waterlogged-scribe.hpp"
#include "../entities/enemy/shard-soldier.hpp"
#include "../entities/enemy/bone-sprinter.hpp"
#include "../entities/effects/slowed-effect.hpp"
#include "../entities/player.hpp"

Chamber::Chamber(Player& player) : player(player), isCompleted(false) {
    player.setChamber(this);
}

void Chamber::update(float dt) {
    for (auto it = enemies.begin(); it != enemies.end(); ) {
        if (!(*it)->isAlive()) {
            (*it)->onDeath();
            if (dropsFragments) {
                itemManager.spawnEnemyFragments((*it).get(), player);
            }
            it = enemies.erase(it);
        } else {
            (*it)->update(dt);
            (*it)->updateState(dt, *this);
            ++it;
        }
    }
    
    itemManager.update(dt, player, *this);
    checkCollisions(dt);
}

void Chamber::draw(sf::RenderWindow& window) {
    window.draw(tileMap);
    
    drawBackground(window);
    
    itemManager.draw(window);
    for (const auto& enemy : enemies) {
        enemy->draw(window);
    }
    
    drawForeground(window);
    
    for (const auto& hb : debugHitboxes) {
        CollisionSolver::drawDebug(window, hb.shape);
    }
    debugHitboxes.clear();
}

const std::vector<std::vector<int>>& Chamber::getGrid() const {
    return grid;
}

std::vector<sf::FloatRect> Chamber::getObstaclesFor(const Character* character) const {
    std::vector<sf::FloatRect> obs = baseObstacles;
    
    if (!character || grid.empty() || grid[0].empty()) return obs;

    sf::FloatRect bounds = character->getBounds();
    sf::Vector2f trueCenter = {bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f}; // True center

    float cellSize = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();

    int tx = static_cast<int>((trueCenter.x - ox) / cellSize);
    int ty = static_cast<int>((trueCenter.y - oy) / cellSize);

    int charLevel = 0; // Default Level 0
    if (ty >= 0 && ty < grid.size() && tx >= 0 && tx < grid[0].size()) {
        int tileType = grid[ty][tx];
        // 4 = Elevated Floor, 5 = Stairs
        // We removed 6 (Cliff face) from charLevel=1 because we want to completely
        // prevent walking onto the cliff face. It should be a wall for both levels.
        if (tileType == 4 || tileType == 5) {
            charLevel = 1;
        }
    }

    if (charLevel == 0) {
        // On ground level: elevated tiles and cliff faces are solid walls.
        obs.insert(obs.end(), elevationObstacles.begin(), elevationObstacles.end());
    } else {
        // On level 1: ground tiles are solid walls, preventing walking off the edge.
        obs.insert(obs.end(), inverseElevationObstacles.begin(), inverseElevationObstacles.end());
    }

    return obs;
}

void Chamber::onFragmentCollected(float value) {
    // Default implementation does nothing
}

void Chamber::onEnemyHit(Enemy* enemy, bool lethal) {
    // Default implementation does nothing
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

void Chamber::setGrid(const std::vector<std::vector<int>>& newGrid) {
    grid = newGrid;
    buildObstaclesFromGrid();
    
    float size = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();
    tileMap = TileMapGenerator::generate(grid, size, ox, oy);
}

void Chamber::buildObstaclesFromGrid() {
    baseObstacles.clear();
    elevationObstacles.clear();

    if (grid.empty() || grid[0].empty()) return;

    float size = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();

    int rows = grid.size();
    int cols = grid[0].size();
    
    // Implicit Map Boundaries
    float mapWidth = cols * size;
    float mapHeight = rows * size;
    float thickness = 1000.f; // arbitrary thick wall to prevent escaping
    
    baseObstacles.push_back(sf::FloatRect({ox - thickness, oy - thickness}, {mapWidth + 2 * thickness, thickness})); // Top
    baseObstacles.push_back(sf::FloatRect({ox - thickness, oy + mapHeight}, {mapWidth + 2 * thickness, thickness})); // Bottom
    baseObstacles.push_back(sf::FloatRect({ox - thickness, oy}, {thickness, mapHeight})); // Left
    baseObstacles.push_back(sf::FloatRect({ox + mapWidth, oy}, {thickness, mapHeight})); // Right

    // Row 0 is always rendered as a wall-front decorative strip (see TileMapGenerator).
    // Block movement into it so characters never walk behind that visual wall.
    baseObstacles.push_back(sf::FloatRect({ox, oy}, {mapWidth, size})); // Top wall-front row

    for (size_t y = 0; y < rows; ++y) {
        for (size_t x = 0; x < cols; ++x) {
            int tileType = grid[y][x];
            sf::FloatRect rect({ox + x * size, oy + y * size}, {size, size});
            
            // 2 = Lake (Impassable for all)
            if (tileType == 2) {
                baseObstacles.push_back(rect);
            }
            // 0 = Ground (Impassable for Level 1 characters)
            else if (tileType == 0) {
                // To allow characters to walk off stairs, the ground directly below
                // stairs should NOT be a barrier for Level 1 characters.
                bool belowStair = false;
                if (y > 0 && grid[y-1][x] == 5) {
                    belowStair = true;
                }
                if (!belowStair) {
                    inverseElevationObstacles.push_back(rect);
                }
            } 
            // 6 = Cliff Face (wall-front) — always an elevation obstacle (Level 0 barrier)
            // It is ALSO an inverse obstacle (Level 1 barrier) so you can't walk off the ledge.
            else if (tileType == 6) {
                elevationObstacles.push_back(rect);
                inverseElevationObstacles.push_back(rect);
            }
            // 4 = Elevated Floor — obstacle for ground-level characters.
            // The ONLY exemption is the single tile directly above a stair (type 5).
            // This keeps the stair approach clear for the character's hitbox
            // (which extends half a cell upward from center, i.e. characterSize/2)
            // while sealing the rest of the island from all other directions.
            else if (tileType == 4) {
                bool directlyAboveStair = false;
                if (y + 1 < rows && grid[y + 1][x] == 5) {
                    directlyAboveStair = true;
                }
                if (!directlyAboveStair) {
                    elevationObstacles.push_back(rect);
                }
            }
            // 5 = Stairs (Walkable ramp, no obstacle)
        }
    }
}

void Chamber::spawnEnemy(std::unique_ptr<Enemy> enemy) {
    enemies.push_back(std::move(enemy));
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


void Chamber::processPlayerAttack(const Hitbox& hitbox) {
    debugHitboxes.push_back({hitbox, 0.2f});
    int killsThisAttack = 0;
    std::vector<Enemy*> killedEnemies;

    for (auto& enemy : enemies) {
        if (!enemy->isAlive()) continue;

        if (CollisionSolver::checkCollision(hitbox, enemy->getBounds())) {
            float hpBefore = enemy->getHp();
            float damage = player.getEffectiveStats().damage;
            bool lethal = (hpBefore - damage) <= 0;
            
            enemy->takeDamage(damage);
            
            if (hpBefore > 0 && enemy->getHp() <= 0) {
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
}