#include "chamber.hpp"
#include "../global-settings/setting-manager.hpp"

Chamber::Chamber(Player& player) : player(player), isCompleted(false) {}

void Chamber::setGrid(const std::vector<std::vector<int>>& newGrid) {
    grid = newGrid;
    buildObstaclesFromGrid();
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

void Chamber::checkCollisions() {
    // Basic collision checks can be added here
}

BaseChamber::BaseChamber(Player& player) : Chamber(player) {}

void BaseChamber::update(float dt) {
    for (auto it = debugHitboxes.begin(); it != debugHitboxes.end(); ) {
        it->timer -= dt;
        if (it->timer <= 0) {
            it = debugHitboxes.erase(it);
        } else {
            ++it;
        }
    }
}

void BaseChamber::draw(sf::RenderWindow& window) {
    for (const auto& hb : debugHitboxes) {
        CollisionSolver::drawDebug(window, hb.shape);
    }
}

void BaseChamber::processPlayerAttack(const Hitbox& hitbox) {
    debugHitboxes.push_back({hitbox, 0.2f});
}
