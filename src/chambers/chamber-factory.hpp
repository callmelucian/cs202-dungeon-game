#ifndef CHAMBER_FACTORY_HPP
#define CHAMBER_FACTORY_HPP

#include <memory>
#include "chamber.hpp"

class Player;

class ChamberFactory {
public:
    static std::unique_ptr<Chamber> createChamber(int level, int chamberIndex, Player& player);
};

#endif // CHAMBER_FACTORY_HPP
