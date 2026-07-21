#include "map-loader.hpp"
#include "setting-manager.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


std::vector<std::vector<int>> MapLoader::loadChamberGrid(int level, int chamberIndex) {
    // 0 = Floor
    // 1 = Outer Wall
    // 2 = Inner Obstacle
    
    unsigned int cols = SettingManager::getInstance().getGridCols();
    unsigned int rows = SettingManager::getInstance().getGridRows();

    std::vector<std::vector<int>> grid(rows, std::vector<int>(cols, 0));
    
    // Grid boundaries will be handled implicitly by Chamber physics.
    
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
    
    // Add elevated block
    auto addElevated = [&](int startX, int startY, int w, int h) {
        int stairCenter = startX + w / 2;
        for (int y = startY; y < startY + h; ++y) {
            for (int x = startX; x < startX + w; ++x) {
                if (y >= 0 && y < rows && x >= 0 && x < cols) {
                    if (y == startY + h - 1) {
                        // Bottom row of elevation block: 2 stair cells centered
                        if (x == stairCenter || x == stairCenter - 1) {
                            grid[y][x] = 5; // Stairs
                        } else {
                            grid[y][x] = 6; // Cliff face
                        }
                    } else {
                        grid[y][x] = 4; // Elevated floor
                    }
                }
            }
        }
    };
    
    addLake(2, 2, 2, 2);
    addElevated(5, 5, 8, 5);
    
    return grid;
}

ChamberConfig MapLoader::loadChamber(const std::string& filepath) {
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
    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error in " << filepath << ": " << e.what() << std::endl;
    }

    return config;
}

std::vector<WaveConfig> MapLoader::loadWaves(const std::string& filepath) {
    std::vector<WaveConfig> waves;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open wave config file: " << filepath << std::endl;
        return waves;
    }

    try {
        json j;
        file >> j;

        if (j.contains("waves") && j["waves"].is_array()) {
            for (const auto& waveObj : j["waves"]) {
                WaveConfig wave;
                wave.enemyType = waveObj.value("enemyType", "unknown");
                wave.count = waveObj.value("count", 1);
                wave.spawnDelay = waveObj.value("spawnDelay", 0.0f);
                waves.push_back(wave);
            }
        }
    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error in " << filepath << ": " << e.what() << std::endl;
    }

    return waves;
}
