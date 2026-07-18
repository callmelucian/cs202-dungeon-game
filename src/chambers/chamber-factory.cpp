#include "chamber-factory.hpp"
#include "test-chamber.hpp"
#include "../global-settings/map-loader.hpp"
#include <iostream>

#include "prevent-chamber.hpp"
#include "protect-chamber.hpp"

std::unique_ptr<Chamber> ChamberFactory::createChamber(ChamberSelectionType type, int level, int chamberIndex, Player& player) {
    std::cout << "Creating Chamber - Level: " << level << ", Index: " << chamberIndex << "\n";
    
    std::unique_ptr<Chamber> chamber;
    
    if (type == ChamberSelectionType::PREVENT) {
        auto preventChamber = std::make_unique<PreventChamber>(player, EchoType::CLARITY_SHARD);
        preventChamber->setExitPosition({500.f, 500.f}); // Arbitrary test coordinates
        chamber = std::move(preventChamber);
    } else if (type == ChamberSelectionType::PROTECT) {
        auto protectChamber = std::make_unique<ProtectChamber>(player, "Test Echo", 10.0f);
        protectChamber->setEchoPosition({300.f, 300.f}); // Arbitrary test coordinates
        chamber = std::move(protectChamber);
    } else {
        chamber = std::make_unique<TestChamber>(player);
    }
    
    std::vector<std::vector<int>> grid = MapLoader::loadChamberGrid(level, chamberIndex);
    chamber->setGrid(grid);

    return chamber;
}
