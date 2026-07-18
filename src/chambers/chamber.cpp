#include "chamber.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../entities/enemy/waterlogged-scribe.hpp"
#include "../entities/enemy/shard-soldier.hpp"
#include "../entities/enemy/bone-sprinter.hpp"
#include "../entities/effects/slowed-effect.hpp"
#include "../entities/player.hpp"

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
