#include "map-loader.hpp"
#include "setting-manager.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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

ChamberConfig MapLoader::loadChamberConfig(const std::string& filepath) {
    ChamberConfig config;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open chamber config file: " << filepath << std::endl;
        return config;
    }

    try {
        json j;
        file >> j;

        config.chamberType = j.value("chamberType", "MidChamber");
        config.width = j.value("width", 20);
        config.height = j.value("height", 20);

        if (j.contains("grid") && j["grid"].is_array()) {
            for (const auto& row : j["grid"]) {
                std::vector<int> rowVec;
                for (const auto& cell : row) {
                    rowVec.push_back(cell.get<int>());
                }
                config.grid.push_back(rowVec);
            }
        }

        if (j.contains("waves") && j["waves"].is_array()) {
            for (const auto& waveObj : j["waves"]) {
                WaveConfig wave;
                wave.enemyType = waveObj.value("enemyType", "unknown");
                wave.count = waveObj.value("count", 1);
                wave.spawnDelay = waveObj.value("spawnDelay", 0.0f);
                config.waves.push_back(wave);
            }
        }
    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error in " << filepath << ": " << e.what() << std::endl;
    }

    return config;
}
