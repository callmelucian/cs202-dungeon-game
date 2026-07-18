#ifndef ECHO_FRAGMENT_HPP
#define ECHO_FRAGMENT_HPP

#include "item.hpp"
#include <SFML/Graphics/CircleShape.hpp>

class EchoFragment : public Item {
public:
    EchoFragment(sf::Vector2f spawnPos);

    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) const override;
    void onCollect(Player& player, Chamber& chamber) override;

    void pullTo(sf::Vector2f targetPos);

private:
    sf::Vector2f velocity;
    bool isMagnetized;
    sf::CircleShape shape;
    
    const float MAGNET_SPEED = 600.0f;
};

#endif // ECHO_FRAGMENT_HPP