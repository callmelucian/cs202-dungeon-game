#include "item.hpp"
#include "../entities/player.hpp"
#include "../chambers/chamber.hpp"
#include <cmath>
#include <random>
#include <iostream>
#include "../global-settings/sound-manager.hpp"
#include "../graphics/particle-system.hpp"

Item::Item(sf::Vector2f startPos)
    : drag(2.0f), magnetRadius(250.0f), collectionRadius(20.0f), lifetime(0.0f), collected(false) {
    
    setPosition(startPos);
    
    // Random scatter velocity
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
    std::uniform_real_distribution<float> speedDist(100.0f, 250.0f);
    
    float angle = angleDist(rng) * 3.14159f / 180.0f;
    float speed = speedDist(rng);
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
        float magnetStrength = 2500.0f;
        sf::Vector2f dir = diff / dist;
        velocity += dir * magnetStrength * dt;
    }
    
    setPosition(getPosition() + velocity * dt);
}

bool Item::isCollected() const {
    return collected;
}

sf::FloatRect Item::getBounds() const {
    // getGlobalBounds() already includes the object's transform/position
    return shape.getGlobalBounds();
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
    SoundManager::getInstance().playSound("pickup");
    ParticleSystem::getInstance().emitSparkle(getPosition(), 15, sf::Color(100, 200, 255), 15.0f);
    chamber.onFragmentCollected(value);
    // In a full implementation, you would update player stats, economy, or momentum here.
}
