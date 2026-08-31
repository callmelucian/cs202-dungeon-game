#ifndef MID_CHAMBER_HPP
#define MID_CHAMBER_HPP

#include "chamber.hpp"
#include <SFML/Graphics.hpp>

class MidChamber : public Chamber {
public:
    MidChamber(Player& player);
    virtual ~MidChamber();

    void update(float dt) override;
    void setExitPosition(const sf::Vector2f& pos) override;

protected:
    void completeChamber() override;
};

#endif // MID_CHAMBER_HPP
