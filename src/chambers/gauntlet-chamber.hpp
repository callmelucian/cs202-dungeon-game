#ifndef GAUNTLET_CHAMBER_HPP
#define GAUNTLET_CHAMBER_HPP

#include "chamber.hpp"
#include "wave-spawner.hpp"

class GauntletChamber : public Chamber {
public:
    GauntletChamber(Player& player);
    virtual ~GauntletChamber() = default;

    void update(float dt) override;
    void drawForeground(sf::RenderWindow& window) override;

    void setIsHungerPit(bool active) { isHungerPit = active; }

private:
    bool isHungerPit = false;
};

#endif // GAUNTLET_CHAMBER_HPP
