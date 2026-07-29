#ifndef MID_CHAMBER_HPP
#define MID_CHAMBER_HPP

#include "chamber.hpp"
#include <SFML/Graphics.hpp>

class MidChamber : public Chamber {
public:
    MidChamber(Player& player);
    virtual ~MidChamber();

    void update(float dt) override;

    // Place the exit trigger zone in world coordinates.
    // When the player steps into this rect, the chamber completes.
    void setExitPosition(sf::Vector2f pos);

protected:
    void completeChamber() override;
    void drawForeground(sf::RenderWindow& window) override;

private:
    sf::Vector2f exitPosition = {-1.f, -1.f}; // unset until factory calls setExitPosition
    sf::RectangleShape exitShape;
};

#endif // MID_CHAMBER_HPP
