#include "pathfinder.hpp"
#include "../global-settings/setting-manager.hpp"
#include <queue>
#include <cmath>
#include <algorithm>

bool Pathfinder::isWalkable(sf::Vector2f position, const std::vector<std::vector<int>>& grid) {
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

std::vector<sf::Vector2f> Pathfinder::findPath(sf::Vector2f start, sf::Vector2f target, const std::vector<std::vector<int>>& grid) {
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
