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

const std::vector<sf::FloatRect>& Chamber::getObstacles() const {
    return obstacles;
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
    obstacles.clear();
    for (size_t y = 0; y < grid.size(); ++y) {
        for (size_t x = 0; x < grid[y].size(); ++x) {
            int tileType = grid[y][x];
            // 1 = Outer Wall, 2 = Inner Obstacle
            if (tileType == 1 || tileType == 2) {
                float size = SettingManager::getInstance().getCellSize();
                float ox = SettingManager::getInstance().getGridOffsetX();
                float oy = SettingManager::getInstance().getGridOffsetY();
                sf::FloatRect rect({ox + x * size, oy + y * size}, {size, size});
                obstacles.push_back(rect);
            }
        }
    }
}

void Chamber::spawnEnemy(std::unique_ptr<Enemy> enemy) {
    enemies.push_back(std::move(enemy));
}

void Chamber::checkCollisions(float dt) {
    // Resolve enemy collisions with obstacles
    for (auto& enemy : enemies) {
        CollisionSolver::resolveAABB(*enemy, obstacles, dt);
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