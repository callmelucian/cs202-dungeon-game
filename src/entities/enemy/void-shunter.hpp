#ifndef VOID_SHUNTER_HPP
#define VOID_SHUNTER_HPP

#include "enemy.hpp"
#include <SFML/System/Vector2.hpp>

class VoidShunter : public Enemy {
public:
    VoidShunter(Player& player);
    
    void updateState(float dt, Chamber& chamber) override;
    void onDeath(Chamber* chamber = nullptr) override;

private:
    bool isCharging;
    float chargeTimer;
    float chargeCooldown;
    sf::Vector2f chargeDir;
};

#endif // VOID_SHUNTER_HPP