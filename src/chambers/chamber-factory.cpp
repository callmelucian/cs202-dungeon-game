#include "chamber-factory.hpp"
#include "test-chamber.hpp"
#include "map-loader.hpp"
#include "../global-settings/setting-manager.hpp"
#include <iostream>

#include "prevent-chamber.hpp"
#include "protect-chamber.hpp"

#include "gauntlet-chamber.hpp"
#include "mid-chamber.hpp"
#include "boss-chamber.hpp"

std::unique_ptr<Chamber> ChamberFactory::createChamber(int level, int chamberIndex, Player& player) {
    std::cout << "Creating Chamber - Level: " << level << ", Index: " << chamberIndex << "\n";
    
    std::string filepath = MapLoader::getChamberFilepath(level, chamberIndex);
    if (filepath.empty()) {
        std::cerr << "Warning: No filepath found for Level " << level << ", Chamber " << chamberIndex << ". Falling back to TestChamber.\n";
        return std::make_unique<TestChamber>(player);
    }
    
    ChamberConfig config = MapLoader::loadChamber(filepath);
    
    std::unique_ptr<Chamber> chamber;
    
    if (config.chamberType == "ProtectChamber") {
        auto protectChamber = std::make_unique<ProtectChamber>(player, "Test Echo", 10.0f);
        // Position will be set from grid later
        chamber = std::move(protectChamber);
    } else if (config.chamberType == "PreventChamber") {
        auto preventChamber = std::make_unique<PreventChamber>(player, EchoType::CLARITY_SHARD);
        preventChamber->setExitPosition({500.f, 500.f}); // Can be extracted from config later
        chamber = std::move(preventChamber);
    } else if (config.chamberType == "GauntletChamber") {
        auto gauntlet = std::make_unique<GauntletChamber>(player);
        if (!config.waves.empty()) {
            gauntlet->setWaves(config.waves);
        }
        chamber = std::move(gauntlet);
    } else if (config.chamberType == "MidChamber") {
        chamber = std::make_unique<MidChamber>(player);
    } else if (config.chamberType == "BossChamber") {
        chamber = std::make_unique<BossChamber>(player);
    } else {
        chamber = std::make_unique<TestChamber>(player);
    }
    
    if (config.playerSpawnCell.first >= 0 && config.playerSpawnCell.second >= 0) {
        float spawnX = config.playerSpawnCell.second + 0.5f;
        float spawnY = config.playerSpawnCell.first + 0.5f;
        chamber->setPlayerSpawn({spawnX, spawnY});
    } else if (config.playerSpawnX >= 0.0f && config.playerSpawnY >= 0.0f) {
        chamber->setPlayerSpawn({config.playerSpawnX, config.playerSpawnY});
    }

    if (!config.typeGrid.empty() && !config.levelGrid.empty()) {
        chamber->setGrids2D5(config.typeGrid, config.levelGrid);
    }

    if (chamber && !config.waves.empty()) {
        chamber->setWaves(config.waves);
    }

    // Place it at the bottom-center of the walkable map area.
    if (config.chamberType == "MidChamber") {
        auto* midChamber = dynamic_cast<MidChamber*>(chamber.get());
        if (midChamber) {
            const auto& typeGrid = chamber->getTypeGrid();
            if (!typeGrid.empty() && !typeGrid[0].empty()) {
                int rows = typeGrid.size();
                int cols = typeGrid[0].size();
                float size = SettingManager::getInstance().getCellSize();
                float ox = SettingManager::getInstance().getGridOffsetX();
                float oy = SettingManager::getInstance().getGridOffsetY();
                
                // Position exit at bottom center of the playable area
                float exitX = ox + (cols / 2.0f) * size;
                float exitY = oy + (rows - 2) * size;
                midChamber->setExitPosition({exitX, exitY});
            }
        }
    } else if (config.chamberType == "PreventChamber") {
        auto* preventChamber = dynamic_cast<PreventChamber*>(chamber.get());
        if (preventChamber) {
            const auto& typeGrid = chamber->getTypeGrid();
            if (!typeGrid.empty() && !typeGrid[0].empty()) {
                int rows = typeGrid.size();
                int cols = typeGrid[0].size();
                float size = SettingManager::getInstance().getCellSize();
                float ox = SettingManager::getInstance().getGridOffsetX();
                float oy = SettingManager::getInstance().getGridOffsetY();
                
                // Position exit at the far right end of the corridor
                float exitX = ox + (cols - 2.5f) * size;
                float exitY = oy + (rows / 2.0f) * size;
                preventChamber->setExitPosition({exitX, exitY});
            }
        }
    } else if (config.chamberType == "ProtectChamber") {
        auto* protectChamber = dynamic_cast<ProtectChamber*>(chamber.get());
        if (protectChamber) {
            const auto& typeGrid = chamber->getTypeGrid();
            bool foundEcho = false;
            if (!typeGrid.empty() && !typeGrid[0].empty()) {
                float size = SettingManager::getInstance().getCellSize();
                float ox = SettingManager::getInstance().getGridOffsetX();
                float oy = SettingManager::getInstance().getGridOffsetY();
                
                for (size_t y = 0; y < typeGrid.size(); ++y) {
                    for (size_t x = 0; x < typeGrid[y].size(); ++x) {
                        if (typeGrid[y][x] == "E") {
                            protectChamber->setEchoPosition({ox + (x + 0.5f) * size, oy + (y + 0.5f) * size});
                            foundEcho = true;
                            break;
                        }
                    }
                    if (foundEcho) break;
                }
            }
            if (!foundEcho) {
                protectChamber->setEchoPosition({300.f, 300.f}); // Fallback
            }
        }
    }

    return chamber;
}

std::unique_ptr<Chamber> ChamberFactory::createDebugChamber(ChamberSelectionType type, Player& player) {
    std::cout << "Creating Debug Chamber\n";
    
    std::unique_ptr<Chamber> chamber;
    
    if (type == ChamberSelectionType::PREVENT) {
        auto preventChamber = std::make_unique<PreventChamber>(player, EchoType::CLARITY_SHARD);
        float size = SettingManager::getInstance().getCellSize();
        float ox = SettingManager::getInstance().getGridOffsetX();
        float oy = SettingManager::getInstance().getGridOffsetY();
        preventChamber->setExitPosition({ox + 27.5f * size, oy + 7.5f * size});
        chamber = std::move(preventChamber);
    } else if (type == ChamberSelectionType::PROTECT) {
        auto protectChamber = std::make_unique<ProtectChamber>(player, "Test Echo", 10.0f);
        protectChamber->setEchoPosition({300.f, 300.f});
        chamber = std::move(protectChamber);
    } else if (type == ChamberSelectionType::BOSS) {
        chamber = std::make_unique<BossChamber>(player);
    } else {
        chamber = std::make_unique<TestChamber>(player);
    }
    
    std::string filepath = MapLoader::getChamberFilepath(1, 1);
    if (filepath.empty()) filepath = "assets/maps/level-1/chamber-1.json";
    ChamberConfig config = MapLoader::loadChamber(filepath);
    
    if (config.playerSpawnCell.first >= 0 && config.playerSpawnCell.second >= 0) {
        float spawnX = config.playerSpawnCell.second + 0.5f;
        float spawnY = config.playerSpawnCell.first + 0.5f;
        chamber->setPlayerSpawn({spawnX, spawnY});
    }

    if (!config.typeGrid.empty() && !config.levelGrid.empty()) {
        chamber->setGrids2D5(config.typeGrid, config.levelGrid);
    }

    return chamber;
}
