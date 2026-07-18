#ifndef ITEM_HPP
#define ITEM_HPP

#include <SFML/Graphics.hpp>

class Player;
class Chamber;

class Item {
public:
    Item(sf::Vector2f position, sf::FloatRect bounds);
    virtual ~Item() = default;

    virtual void update(float deltaTime) = 0;
    virtual void draw(sf::RenderWindow& window) const = 0;
    
    virtual void onCollect(Player& player, Chamber& chamber) = 0;

    sf::Vector2f getPosition() const;
    void setPosition(sf::Vector2f pos);

    sf::FloatRect getBounds() const;

protected:
    sf::Vector2f position;
    sf::FloatRect bounds;
};

#endif // ITEM_HPP