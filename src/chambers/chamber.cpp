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

bool Chamber::isWalkable(sf::Vector2f position) const {
    if (grid.empty() || grid[0].empty()) return false;
    
    float size = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();
    
    int x = static_cast<int>(std::floor((position.x - ox) / size));
    int y = static_cast<int>(std::floor((position.y - oy) / size));
    
    if (y >= 0 && y < grid.size() && x >= 0 && x < grid[y].size()) {
        return grid[y][x] == 0; // 0 is floor
    }
    return false;
}

std::vector<sf::Vector2f> Chamber::findPath(sf::Vector2f start, sf::Vector2f target) const {
    if (grid.empty() || grid[0].empty()) return {};

    float size = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();
    
    int startX = static_cast<int>(std::floor((start.x - ox) / size));
    int startY = static_cast<int>(std::floor((start.y - oy) / size));
    int targetX = static_cast<int>(std::floor((target.x - ox) / size));
    int targetY = static_cast<int>(std::floor((target.y - oy) / size));
    
    if (startY < 0 || startY >= grid.size() || startX < 0 || startX >= grid[0].size()) return {};
    if (targetY < 0 || targetY >= grid.size() || targetX < 0 || targetX >= grid[0].size()) return {};
    if (grid[targetY][targetX] != 0) {
        return {}; // target is unreachable/unwalkable
    }

    if (startX == targetX && startY == targetY) {
        return {target};
    }

    std::vector<std::vector<bool>> visited(grid.size(), std::vector<bool>(grid[0].size(), false));
    std::vector<std::vector<sf::Vector2i>> parent(grid.size(), std::vector<sf::Vector2i>(grid[0].size(), {-1, -1}));
    
    std::queue<sf::Vector2i> q;
    q.push({startX, startY});
    visited[startY][startX] = true;
    
    const int dx[] = {0, 1, 0, -1};
    const int dy[] = {-1, 0, 1, 0};
    
    bool found = false;
    while (!q.empty()) {
        sf::Vector2i curr = q.front();
        q.pop();
        
        if (curr.x == targetX && curr.y == targetY) {
            found = true;
            break;
        }
        
        for (int i = 0; i < 4; ++i) {
            int nx = curr.x + dx[i];
            int ny = curr.y + dy[i];
            
            if (ny >= 0 && ny < grid.size() && nx >= 0 && nx < grid[0].size()) {
                if (!visited[ny][nx] && grid[ny][nx] == 0) {
                    visited[ny][nx] = true;
                    parent[ny][nx] = curr;
                    q.push({nx, ny});
                }
            }
        }
    }
    
    if (!found) return {};
    
    std::vector<sf::Vector2f> path;
    sf::Vector2i curr = {targetX, targetY};
    while (curr.x != -1 && curr.y != -1) {
        path.push_back({ox + curr.x * size + size / 2.0f, oy + curr.y * size + size / 2.0f});
        curr = parent[curr.y][curr.x];
    }
    
    std::reverse(path.begin(), path.end());
    if (!path.empty()) path.erase(path.begin());
    
    if (!path.empty()) {
        path.back() = target;
    }
    
    return path;
}
