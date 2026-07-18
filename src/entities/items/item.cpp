#include "item.hpp"

Item::Item(sf::Vector2f position, sf::FloatRect bounds) : position(position), bounds(bounds) {}

sf::Vector2f Item::getPosition() const {
    return position;
}

void Item::setPosition(sf::Vector2f pos) {
    position = pos;
    bounds.left = position.x - bounds.width / 2.0f;
    bounds.top = position.y - bounds.height / 2.0f;
}

sf::FloatRect Item::getBounds() const {
    return bounds;
}