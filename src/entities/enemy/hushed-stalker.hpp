#ifndef HUSHED_STALKER_HPP
#define HUSHED_STALKER_HPP

#include "enemy.hpp"

class HushedStalker : public Enemy {
public:
    HushedStalker(Player& player);
    
    void updateState(float dt, Chamber& chamber) override;
    void draw(sf::RenderWindow& window) const override;
    void onDeath(Chamber* chamber = nullptr) override;

private:
    float windupTimer;
    bool windingUp;
    bool isVisible;
};

#endif // HUSHED_STALKER_HPP
