#ifndef BONE_SPRINTER_HPP
#define BONE_SPRINTER_HPP

#include "enemy.hpp"

class BoneSprinter : public Enemy {
private:
    bool isCarrier;

public:
    BoneSprinter(Player& player, bool isCarrier = false);
    
    void onDeath(Chamber* chamber = nullptr) override;
};

#endif // BONE_SPRINTER_HPP