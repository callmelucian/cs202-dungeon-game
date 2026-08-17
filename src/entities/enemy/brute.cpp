#include "brute.hpp"
#include "enemy-state.hpp"
#include "enemy-steering-strategy.hpp"
#include "../player.hpp"
#include "../../graphics/particle-system.hpp"
#include <iostream>

Brute::Brute(Player& player) : Enemy("brute", player) {
    baseStats.maxHp = 45.0f;
    baseStats.hp = 45.0f;
    baseStats.damage = 14.0f;
    baseStats.speed = 2.0f;

    setSteeringStrategy(std::make_unique<SeekStrategy>());
    changeState(std::make_unique<ChasingState>());
}

void Brute::onDeath(Chamber* chamber) {
    std::cout << "Brute destroyed! Emitting shockwave...\n";
    ParticleSystem::getInstance().emitBurst(getPosition(), 45, sf::Color(220, 100, 50, 220), 80.0f, 200.0f, 0.4f, 1.0f, 8.0f);
}
