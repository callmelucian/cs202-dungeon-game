#ifndef WATERLOGGED_SCRIBE_HPP
#define WATERLOGGED_SCRIBE_HPP

#include "enemy.hpp"

class WaterloggedScribe : public Enemy {
public:
    WaterloggedScribe(Player& player);
    
    void draw(sf::RenderWindow& window) const override;
    void onDeath() override;
};

#endif // WATERLOGGED_SCRIBE_HPP