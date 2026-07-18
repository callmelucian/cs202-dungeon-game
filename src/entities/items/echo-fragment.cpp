#include "echo-fragment.hpp"
#include "../player.hpp"
#include <iostream>
#include <cmath>
#include <cstdlib>

EchoFragment::EchoFragment(sf::Vector2f spawnPos) : Item(spawnPos, sf::FloatRect({0.f, 0.f}, {10.f, 10.f})), isMagnetized(false) {
    shape.setRadius(5.0f);
    shape.setFillColor(sf::Color(100, 255, 255)); // Cyan for Echo Fragments
    shape.setOrigin({5.0f, 5.0f});
    shape.setPosition(position);
    // May add scatter burst later
    velocity = sf::Vector2f(0.f, 0.f);
}

void EchoFragment::update(float deltaTime) {
    setPosition(position + velocity * deltaTime);
    shape.setPosition(position);
}

void EchoFragment::draw(sf::RenderWindow& window) const {
    window.draw(shape);
}

void EchoFragment::onCollect(Player& player, Chamber& chamber) {
    // TODO: Add logic for form-specific fragment multipliers and UI hooks
    std::cout << "[EchoFragment] Collected!\n";
    markCollected();
}

void EchoFragment::pullTo(sf::Vector2f targetPos) {
    isMagnetized = true;
    
    sf::Vector2f dir = targetPos - position;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    
    if (len > 0) dir /= len;
    
    velocity = dir * MAGNET_SPEED;
}