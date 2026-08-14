#include "choir-husk.hpp"
#include "enemy-state.hpp"
#include "enemy-steering-strategy.hpp"
#include "../../chambers/chamber.hpp"
#include "../player.hpp"
#include "../../global-settings/setting-manager.hpp"

#include <cmath>
#include <iostream>

ChoirHusk::ChoirHusk(Player& player, HuskVariant variant) 
    : Enemy("choir_husk", player), variant(variant), windingUp(false), windupTimer(0.0f), callCooldown(0.0f) {
    
    if (variant == HuskVariant::CARRIER) {
        baseStats.hp = baseStats.maxHp = 16.0f;
        fragmentDropCount = 3;
        setIsRealCarrier(true);
    } else if (variant == HuskVariant::GUARD) {
        baseStats.hp = baseStats.maxHp = 22.0f;
        fragmentDropCount = 1;
    } else {
        baseStats.hp = baseStats.maxHp = 20.0f;
        fragmentDropCount = 1;
    }
    
    baseStats.damage = 7.0f;
    baseStats.speed = 4.0f;

    setSteeringStrategy(std::make_unique<SeekStrategy>());
    changeState(std::make_unique<IdleState>());
}

void ChoirHusk::startWindup() {
    windingUp = true;
    windupTimer = 0.6f;
    setVelocity(sf::Vector2f(0.0f, 0.0f)); // Stop moving during windup
}

bool ChoirHusk::isWindingUp() const {
    return windingUp;
}

void ChoirHusk::triggerCallResponse(Chamber& chamber) {
    int syncedCount = 0;
    sf::Vector2f myPos = getPosition();
    float cellSize = SettingManager::getInstance().getCellSize();

    // Find up to 2 nearby Husks to join the call
    for (Enemy* e : chamber.getEnemiesRaw()) {
        if (e != this && e->isAlive()) {
            ChoirHusk* husk = dynamic_cast<ChoirHusk*>(e);
            if (husk && !husk->isWindingUp()) {
                sf::Vector2f otherPos = husk->getPosition();
                float dist = std::sqrt(std::pow(myPos.x - otherPos.x, 2) + std::pow(myPos.y - otherPos.y, 2));
                
                if (dist < 8.0f * cellSize) { // Synced attack radius (8 units)
                    husk->startWindup();
                    syncedCount++;
                    if (syncedCount >= 2) break;
                }
            }
        }
    }
    
    startWindup(); 
    callCooldown = 3.0f; // Put the call on cooldown so it doesn't spam
}

void ChoirHusk::updateState(float dt, Chamber& chamber) {
    if (callCooldown > 0.0f) {
        callCooldown -= dt;
    }

    if (windingUp) {
        windupTimer -= dt;
        if (windupTimer <= 0.0f) {
            windingUp = false;
            changeState(std::make_unique<AttackingState>());
            attackCooldown = 1.5f; // Wait a bit before attacking/calling again
        }
        return; // Don't run normal AI while winding up
    }

    // Determine if we should trigger a group attack
    sf::Vector2f myPos = getPosition();
    sf::Vector2f playerPos = getPlayer().getPosition();
    float distToPlayer = std::sqrt(std::pow(myPos.x - playerPos.x, 2) + std::pow(myPos.y - playerPos.y, 2));
    float cellSize = SettingManager::getInstance().getCellSize();

    // If close enough to hit the player (2.5 units) and cooldown is ready, make the call!
    if (distToPlayer < 2.5f * cellSize && callCooldown <= 0.0f && attackCooldown <= 0.0f) {
        triggerCallResponse(chamber);
        return;
    }

    Enemy::updateState(dt, chamber);
}

void ChoirHusk::onDeath(Chamber* chamber) {
    std::cout << "ChoirHusk died!\n";
}