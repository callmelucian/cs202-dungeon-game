#ifndef CHAMBER_FACTORY_HPP
#define CHAMBER_FACTORY_HPP

#include <memory>
#include "chamber.hpp"

class Player;

enum class ChamberSelectionType {
    TEST,
    PROTECT,
    PREVENT
};

class ChamberFactory {
public:
    static std::unique_ptr<Chamber> createChamber(ChamberSelectionType type, int level, int chamberIndex, Player& player);
};

#endif // CHAMBER_FACTORY_HPP
