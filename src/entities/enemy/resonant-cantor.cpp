#include "resonant-cantor.hpp"
#include "enemy-state.hpp"
#include "../effects/slowed-effect.hpp"
#include "../player.hpp"
#include "../../chambers/chamber.hpp"
#include <cmath>
#include <iostream>

ResonantCantor::ResonantCantor(Player& player) : Enemy("resonant_cantor", player), pulseTimer(5.0f) {
    baseStats.hp = 50.0f;
    baseStats.maxHp = 50.0f;
    baseStats.damage = 9.0f;
    baseStats.speed = 0.0f; // Does not move
    fragmentDropCount = 1;

    // It remains stationary
    changeState(std::make_unique<IdleState>());
}

void ResonantCantor::onDeath(Chamber* chamber) {
    std::cout << "ResonantCantor died!\n";
}

void ResonantCantor::updateState(float dt, Chamber& chamber) {
    pulseTimer -= dt;
    if (pulseTimer <= 0.0f) {
        emitPulse();
        pulseTimer = 5.0f; // Reset timer
    }
    
    // Standard logic
    Enemy::updateState(dt, chamber);
}

void ResonantCantor::emitPulse() {
    Player& player = getPlayer();
    sf::Vector2f myPos = getPosition();
    sf::Vector2f playerPos = player.getPosition();
    
    float dist = std::hypot(myPos.x - playerPos.x, myPos.y - playerPos.y);
    if (dist <= 6.0f) {
        // Applies SlowedEffect. If already slowed, the Character class handles refreshing/stacking it
        player.applyStatusEffect(std::make_unique<SlowedEffect>());
    }
}
