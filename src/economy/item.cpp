#include "item.hpp"
#include "../entities/player.hpp"
#include "../chambers/chamber.hpp"
#include "../global-settings/asset-manager.hpp"
#include "../global-settings/sound-manager.hpp"
#include "../graphics/particle-system.hpp"
#include "../entities/effects/speed-up-effect.hpp"
#include <cmath>
#include <random>
#include <iostream>

Item::Item(sf::Vector2f startPos, sf::IntRect textureRect)
    : drag(2.0f),
      magnetRadius(250.0f),
      collectionRadius(20.0f),
      lifetime(0.0f),
      collected(false),
      sprite(AssetManager::getInstance().getTexture("items"), textureRect) {
    
    sprite.setOrigin(sf::Vector2f(12.0f, 12.0f));
    setPosition(startPos);
    
    // Random scatter velocity
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
    std::uniform_real_distribution<float> speedDist(100.0f, 250.0f);
    
    float angle = angleDist(rng) * 3.14159f / 180.0f;
    float speed = speedDist(rng);
    velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
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
    sf::Vector2f pos = getPosition();
    return sf::FloatRect({pos.x - 12.0f, pos.y - 12.0f}, {24.0f, 24.0f});
}

void Item::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    target.draw(sprite, states);
}

// ---- EchoFragment ----
EchoFragment::EchoFragment(sf::Vector2f startPos, float value)
    : Item(startPos, sf::IntRect({120, 96}, {24, 24})), value(value) {}

void EchoFragment::onCollect(Player& player, Chamber& chamber) {
    std::cout << "Echo fragment collected! Value: " << value << std::endl;
    SoundManager::getInstance().playSound("pickup");
    ParticleSystem::getInstance().emitSparkle(getPosition(), 15, sf::Color(100, 200, 255), 15.0f);
    chamber.onFragmentCollected(value);
}

// ---- FreezePotion ----
FreezePotion::FreezePotion(sf::Vector2f startPos)
    : Item(startPos, sf::IntRect({48, 240}, {24, 24})) {}

void FreezePotion::onCollect(Player& player, Chamber& chamber) {
    std::cout << "Freeze Potion collected! Freezing all enemies for 10 seconds.\n";
    SoundManager::getInstance().playSound("pickup");
    ParticleSystem::getInstance().emitBurst(getPosition(), 25, sf::Color(100, 220, 255, 230), 40.0f, 120.0f, 0.4f, 0.9f, 4.0f);
    chamber.freezeAllEnemies(10.0f);
}

// ---- SpeedPotion ----
SpeedPotion::SpeedPotion(sf::Vector2f startPos)
    : Item(startPos, sf::IntRect({96, 240}, {24, 24})) {}

void SpeedPotion::onCollect(Player& player, Chamber& chamber) {
    std::cout << "Speed-Up Potion collected! +50% speed for 10 seconds.\n";
    SoundManager::getInstance().playSound("pickup");
    ParticleSystem::getInstance().emitBurst(getPosition(), 25, sf::Color(255, 215, 0, 230), 40.0f, 120.0f, 0.4f, 0.9f, 4.0f);
    player.applyStatusEffect(std::make_unique<SpeedUpEffect>(10.0f));
}

// ---- AntidotePotion ----
AntidotePotion::AntidotePotion(sf::Vector2f startPos)
    : Item(startPos, sf::IntRect({144, 240}, {24, 24})) {}

void AntidotePotion::onCollect(Player& player, Chamber& chamber) {
    std::cout << "Antidote Potion collected! Purging negative status effects.\n";
    SoundManager::getInstance().playSound("pickup");
    ParticleSystem::getInstance().emitBurst(getPosition(), 25, sf::Color(50, 255, 120, 230), 40.0f, 120.0f, 0.4f, 0.9f, 4.0f);
    player.clearNegativeStatusEffects();
}

// ---- HealthPotion (Healer) ----
HealthPotion::HealthPotion(sf::Vector2f startPos)
    : Item(startPos, sf::IntRect({48, 216}, {24, 24})) {}

void HealthPotion::onCollect(Player& player, Chamber& chamber) {
    float healAmount = player.getEffectiveStats().maxHp * 0.10f;
    player.heal(healAmount);
    std::cout << "Healer collected! Restored +10% Max HP (" << healAmount << " HP).\n";
    SoundManager::getInstance().playSound("pickup");
    ParticleSystem::getInstance().emitBurst(getPosition(), 25, sf::Color(255, 80, 100, 230), 40.0f, 120.0f, 0.4f, 0.9f, 4.0f);
}
