#include "chamber-factory.hpp"
#include "test-chamber.hpp"
#include "../global-settings/map-loader.hpp"
#include <iostream>

#include "prevent-chamber.hpp"
#include "protect-chamber.hpp"

#include "gauntlet-chamber.hpp"
#include "mid-chamber.hpp"

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
        protectChamber->setEchoPosition({300.f, 300.f}); // Can be extracted from config later
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
    } else {
        chamber = std::make_unique<TestChamber>(player);
    }
    
    if (config.playerSpawnX >= 0.0f && config.playerSpawnY >= 0.0f) {
        chamber->setPlayerSpawn({config.playerSpawnX, config.playerSpawnY});
    }

    if (!config.grid.empty()) {
        chamber->setGrid(config.grid);
    } else {
        std::vector<std::vector<int>> defaultGrid = MapLoader::loadChamberGrid(level, chamberIndex);
        chamber->setGrid(defaultGrid);
    }

    return chamber;
}

std::unique_ptr<Chamber> ChamberFactory::createDebugChamber(ChamberSelectionType type, Player& player) {
    std::cout << "Creating Debug Chamber\n";
    
    std::unique_ptr<Chamber> chamber;
    
    if (type == ChamberSelectionType::PREVENT) {
        auto preventChamber = std::make_unique<PreventChamber>(player, EchoType::CLARITY_SHARD);
        preventChamber->setExitPosition({500.f, 500.f});
        chamber = std::move(preventChamber);
    } else if (type == ChamberSelectionType::PROTECT) {
        auto protectChamber = std::make_unique<ProtectChamber>(player, "Test Echo", 10.0f);
        protectChamber->setEchoPosition({300.f, 300.f});
        chamber = std::move(protectChamber);
    } else {
        chamber = std::make_unique<TestChamber>(player);
    }
    
    std::vector<std::vector<int>> grid = MapLoader::loadChamberGrid(1, 1);
    chamber->setGrid(grid);

    return chamber;
}
