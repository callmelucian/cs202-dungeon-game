#include "map-loader.hpp"
#include "../global-settings/setting-manager.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;




#include <filesystem>

static std::string resolveFilePath(const std::string& path) {
    if (std::filesystem::exists(path)) return path;
    if (std::filesystem::exists("../" + path)) return "../" + path;
    return path;
}

ChamberConfig MapLoader::loadChamber(const std::string& filepath) {
    ChamberConfig config;
    std::string resolvedPath = resolveFilePath(filepath);
    std::ifstream file(resolvedPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open chamber config file: " << filepath << " (resolved: " << resolvedPath << ")\n";
        return config;
    }

    try {
        json j;
        file >> j;

        config.chamberType = j.value("chamberType", "MidChamber");
        config.width = j.value("width", 20);
        config.height = j.value("height", 20);

        if (j.contains("playerSpawn")) {
            if (j["playerSpawn"].is_object()) {
                config.playerSpawnX = j["playerSpawn"].value("x", -1.0f);
                config.playerSpawnY = j["playerSpawn"].value("y", -1.0f);
            } else if (j["playerSpawn"].is_array() && j["playerSpawn"].size() >= 2) {
                int spawnRow = j["playerSpawn"][0].get<int>();
                int spawnCol = j["playerSpawn"][1].get<int>();
                config.playerSpawnCell = {spawnRow, spawnCol};
                config.playerSpawnX = static_cast<float>(spawnCol);
                config.playerSpawnY = static_cast<float>(spawnRow);
            }
        }

        if (j.contains("type-grid") && j["type-grid"].is_array()) {
            for (const auto& row : j["type-grid"]) {
                std::vector<std::string> rowVec;
                for (const auto& cell : row) {
                    rowVec.push_back(cell.get<std::string>());
                }
                config.typeGrid.push_back(rowVec);
            }
        }

        if (j.contains("level-grid") && j["level-grid"].is_array()) {
            for (const auto& row : j["level-grid"]) {
                std::vector<int> rowVec;
                for (const auto& cell : row) {
                    rowVec.push_back(cell.get<int>());
                }
                config.levelGrid.push_back(rowVec);
            }
        }


        
        config.waves = loadWaves(filepath);
    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error in " << filepath << ": " << e.what() << std::endl;
    }

    return config;
}

std::vector<WaveConfig> MapLoader::loadWaves(const std::string& filepath) {
    std::vector<WaveConfig> waves;
    std::string resolvedPath = resolveFilePath(filepath);
    std::ifstream file(resolvedPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open wave config file: " << filepath << " (resolved: " << resolvedPath << ")\n";
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

CampaignConfig MapLoader::loadCampaign(const std::string& filepath) {
    CampaignConfig config;
    std::string resolvedPath = resolveFilePath(filepath);
    std::ifstream file(resolvedPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open campaign config file: " << filepath << " (resolved: " << resolvedPath << ")\n";
        return config;
    }

    try {
        json j;
        file >> j;

        if (j.contains("levels") && j["levels"].is_array()) {
            for (const auto& levelObj : j["levels"]) {
                CampaignLevelConfig lvl;
                lvl.levelIndex = levelObj.value("levelIndex", 1);
                lvl.name = levelObj.value("name", "Unknown Level");
                if (levelObj.contains("chambers") && levelObj["chambers"].is_array()) {
                    for (const auto& chamberObj : levelObj["chambers"]) {
                        lvl.chambers.push_back(chamberObj.value("file", ""));
                    }
                }
                config.levels.push_back(lvl);
            }
        }
    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error in " << filepath << ": " << e.what() << std::endl;
    }

    return config;
}

std::string MapLoader::getChamberFilepath(int level, int chamberIndex) {
    CampaignConfig campaign = loadCampaign("assets/maps/campaign.json");
    for (const auto& lvl : campaign.levels) {
        if (lvl.levelIndex == level) {
            // chamberIndex is 1-based
            if (chamberIndex >= 1 && chamberIndex <= static_cast<int>(lvl.chambers.size())) {
                return lvl.chambers[chamberIndex - 1];
            }
        }
    }
    return "";
}
