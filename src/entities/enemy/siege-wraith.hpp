#ifndef SIEGE_WRAITH_HPP
#define SIEGE_WRAITH_HPP

#include "enemy.hpp"

class SiegeWraith : public Enemy {
public:
    SiegeWraith(Player& player);
    
    void onDeath(Chamber* chamber = nullptr) override;

private:
    void explode(Chamber& chamber);
};

#endif // SIEGE_WRAITH_HPP