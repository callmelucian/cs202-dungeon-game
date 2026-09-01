#include "pathfinder.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../chambers/chamber.hpp"
#include "math-utility.hpp"
#include <queue>
#include <cmath>
#include <algorithm>

bool Pathfinder::isWalkable(sf::Vector2f position, const std::vector<std::vector<std::string>>& grid) {
    if (grid.empty() || grid[0].empty()) return false;
    
    float size = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();
    
    int x = static_cast<int>(std::floor((position.x - ox) / size));
    int y = static_cast<int>(std::floor((position.y - oy) / size));
    
    if (y >= 0 && y < grid.size() && x >= 0 && x < grid[y].size()) {
        std::string cell = grid[y][x];
        return (cell == "L" || cell == "V" || cell == "H" || cell == "S" || cell == "E" || cell == "X");
    }
    return false;
}

std::vector<sf::Vector2f> Pathfinder::findPath(sf::Vector2f start, sf::Vector2f target, const std::vector<std::vector<std::string>>& grid) {
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
    auto isWalkableCell = [](const std::string& cell) {
        return (cell == "L" || cell == "V" || cell == "H" || cell == "S" || cell == "E" || cell == "X");
    };
    if (!isWalkableCell(grid[targetY][targetX])) {
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
                std::string cell = grid[ny][nx];
                if (!visited[ny][nx] && isWalkableCell(cell)) {
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

std::vector<sf::Vector2f> Pathfinder::findPath(sf::Vector2f start, sf::Vector2f target, const Chamber& chamber) {
    const auto& walkableGrid = chamber.getWalkableGrid();
    if (walkableGrid.empty() || walkableGrid[0].empty()) {
        return findPath(start, target, chamber.getTypeGrid());
    }

    float size = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();

    int rows = static_cast<int>(walkableGrid.size());
    int cols = static_cast<int>(walkableGrid[0].size());

    int startX = std::clamp(static_cast<int>(std::floor((start.x - ox) / size)), 0, cols - 1);
    int startY = std::clamp(static_cast<int>(std::floor((start.y - oy) / size)), 0, rows - 1);
    int targetX = std::clamp(static_cast<int>(std::floor((target.x - ox) / size)), 0, cols - 1);
    int targetY = std::clamp(static_cast<int>(std::floor((target.y - oy) / size)), 0, rows - 1);

    // If start cell is unwalkable (mask == 0), resolve to nearest walkable neighbor
    if (walkableGrid[startY][startX] == 0) {
        bool resolved = false;
        for (int r = 1; r <= 2 && !resolved; ++r) {
            for (int dy = -r; dy <= r && !resolved; ++dy) {
                for (int dx = -r; dx <= r && !resolved; ++dx) {
                    int ny = startY + dy;
                    int nx = startX + dx;
                    if (ny >= 0 && ny < rows && nx >= 0 && nx < cols && walkableGrid[ny][nx] != 0) {
                        startX = nx;
                        startY = ny;
                        resolved = true;
                    }
                }
            }
        }
    }

    // If target cell is unwalkable (mask == 0), resolve to nearest walkable neighbor
    if (walkableGrid[targetY][targetX] == 0) {
        bool resolved = false;
        for (int r = 1; r <= 2 && !resolved; ++r) {
            for (int dy = -r; dy <= r && !resolved; ++dy) {
                for (int dx = -r; dx <= r && !resolved; ++dx) {
                    int ny = targetY + dy;
                    int nx = targetX + dx;
                    if (ny >= 0 && ny < rows && nx >= 0 && nx < cols && walkableGrid[ny][nx] != 0) {
                        targetX = nx;
                        targetY = ny;
                        resolved = true;
                    }
                }
            }
        }
    }

    if (startX == targetX && startY == targetY) {
        return {target};
    }

    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
    std::vector<std::vector<sf::Vector2i>> parent(rows, std::vector<sf::Vector2i>(cols, {-1, -1}));

    std::queue<sf::Vector2i> q;
    q.push({startX, startY});
    visited[startY][startX] = true;

    // Directions and corresponding bitmask flags in walkableGrid:
    // 0: Up (dy = -1, dx = 0) -> Bit 0 (1)
    // 1: Right (dy = 0, dx = 1) -> Bit 1 (2)
    // 2: Down (dy = 1, dx = 0) -> Bit 2 (4)
    // 3: Left (dy = 0, dx = -1) -> Bit 3 (8)
    const int dx[] = {0, 1, 0, -1};
    const int dy[] = {-1, 0, 1, 0};
    const uint8_t maskFlags[] = {1, 2, 4, 8};

    bool found = false;
    while (!q.empty()) {
        sf::Vector2i curr = q.front();
        q.pop();

        if (curr.x == targetX && curr.y == targetY) {
            found = true;
            break;
        }

        uint8_t currMask = walkableGrid[curr.y][curr.x];

        for (int i = 0; i < 4; ++i) {
            if ((currMask & maskFlags[i]) != 0) {
                int nx = curr.x + dx[i];
                int ny = curr.y + dy[i];

                if (ny >= 0 && ny < rows && nx >= 0 && nx < cols && !visited[ny][nx]) {
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
    if (!path.empty() && Math::distance(start, path.front()) < 8.0f) {
        path.erase(path.begin());
    }

    if (!path.empty()) {
        path.back() = target;
    }

    return path;
}
