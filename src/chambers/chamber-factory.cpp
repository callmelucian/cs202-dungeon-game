#include "chamber-factory.hpp"
#include "test-chamber.hpp"
#include "../global-settings/map-loader.hpp"
#include <iostream>

std::unique_ptr<Chamber> ChamberFactory::createChamber(int level, int chamberIndex, Player& player) {
    std::cout << "Creating Chamber - Level: " << level << ", Index: " << chamberIndex << "\n";
    // For testing, return a TestChamber instead
    std::unique_ptr<Chamber> chamber = std::make_unique<TestChamber>(player);
    std::cerr << "Got here 1" << std::endl;
    std::vector<std::vector<int>> grid = MapLoader::getInstance().loadChamberGrid(level, chamberIndex);
    std::cerr << "Got here 2" << std::endl;
    chamber->setGrid(grid);

    std::cerr << "Chamber created" << std::endl;
    return chamber;
}
