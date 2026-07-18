#include "item.hpp"
#include "../entities/player.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>

Item::Item(sf::Vector2f startPos)
    : drag(2.0f), magnetRadius(150.0f), collectionRadius(20.0f), lifetime(0.0f), collected(false) {
    
    setPosition(startPos);
    
    // Random scatter velocity
    float angle = (std::rand() % 360) * 3.14159f / 180.0f;
    float speed = 100.0f + (std::rand() % 150);
    velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
    
    // Default visual
    shape.setSize(sf::Vector2f(8.0f, 8.0f));
    shape.setOrigin(sf::Vector2f(4.0f, 4.0f));
    shape.setFillColor(sf::Color::Cyan);
}

void Item::update(float dt, const sf::Vector2f& playerPos) {
    if (collected) return;
    
    lifetime += dt;
    
    // Apply drag (pseudo-gravity/friction)
    velocity.x -= velocity.x * drag * dt;
    velocity.y -= velocity.y * drag * dt;
    
    // Collection Magnet
    sf::Vector2f diff = playerPos - getPosition();
    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    
    if (dist < collectionRadius) {
        collected = true;
    } else if (dist < magnetRadius) {
        // Accelerate towards player
        float magnetStrength = 800.0f;
        sf::Vector2f dir = diff / dist;
        velocity += dir * magnetStrength * dt;
    }
    
    setPosition(getPosition() + velocity * dt);
}

bool Item::isCollected() const {
    return collected;
}

sf::FloatRect Item::getBounds() const {
    sf::FloatRect bounds = shape.getGlobalBounds();
    bounds.position.x += getPosition().x;
    bounds.position.y += getPosition().y;
    return bounds;
}

void Item::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    target.draw(shape, states);
}

// ---- EchoFragment ----

EchoFragment::EchoFragment(sf::Vector2f startPos, float value)
    : Item(startPos), value(value) {
    shape.setFillColor(sf::Color(100, 200, 255)); // Bright blue crystal color
}

void EchoFragment::onCollect(Player& player, Chamber& chamber) {
    std::cout << "Echo fragment collected! Value: " << value << std::endl;
    // In a full implementation, you would update player stats, economy, or momentum here.
}
