#include "map-loader.hpp"
#include "setting-manager.hpp"

MapLoader& MapLoader::getInstance() {
    static MapLoader instance;
    return instance;
}

std::vector<std::vector<int>> MapLoader::loadChamberGrid(int level, int chamberIndex) {
    // 0 = Floor
    // 1 = Outer Wall
    // 2 = Inner Obstacle
    
    unsigned int cols = SettingManager::getInstance().getGridCols();
    unsigned int rows = SettingManager::getInstance().getGridRows();

    std::vector<std::vector<int>> grid(rows, std::vector<int>(cols, 0));
    
    // Top and Bottom walls
    for (unsigned int x = 0; x < cols; ++x) {
        grid[0][x] = 1;
        grid[rows - 1][x] = 1;
    }
    
    // Left and Right walls
    for (unsigned int y = 0; y < rows; ++y) {
        grid[y][0] = 1;
        grid[y][cols - 1] = 1;
    }
    
    // Some inner obstacles (lakes must be at least 2x2 for proper rendering)
    auto addLake = [&](int startX, int startY, int w, int h) {
        for (int y = startY; y < startY + h; ++y) {
            for (int x = startX; x < startX + w; ++x) {
                if (y >= 0 && y < rows && x >= 0 && x < cols) {
                    grid[y][x] = 2; // Lake
                }
            }
        }
    };
    
    addLake(10, 10, 2, 2);
    addLake(15, 25, 4, 3);
    
    return grid;
}
