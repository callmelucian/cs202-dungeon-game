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
    
    // Some inner obstacles
    std::vector<std::pair<int,int>> obstacles = {
        {10, 10},
        {10, 11},
        {11, 10},
        {28, 15},
        {28, 16},
        {28, 17},
        {29, 16},
        {30, 16}
    };
    for (auto [x, y] : obstacles)
        if (0 <= x && x < rows && 0 <= y && y < cols) grid[x][y] = 2;
    
    return grid;
}
