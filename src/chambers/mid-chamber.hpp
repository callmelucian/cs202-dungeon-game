#ifndef MID_CHAMBER_HPP
#define MID_CHAMBER_HPP

#include "chamber.hpp"

class MidChamber : public Chamber {
public:
    MidChamber(Player& player);
    
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
};

#endif // MID_CHAMBER_HPP
