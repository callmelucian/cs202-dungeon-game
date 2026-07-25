#include "void-shunter.hpp"
#include "enemy-state.hpp"
#include "enemy-steering-strategy.hpp"
#include "../../chambers/chamber.hpp"
#include "../player.hpp"
#include "../../utils/collision-solver.hpp"
#include "../../utils/math-utility.hpp"
#include "../../global-settings/setting-manager.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

VoidShunter::VoidShunter(Player& player) 
    : Enemy("void_shunter", player), isCharging(false), chargeTimer(0.0f), chargeCooldown(2.0f), chargeDir(0.f, 0.f) {
    
    baseStats.hp = 25.0f;
    baseStats.maxHp = 25.0f;
    baseStats.damage = 0.0f; // Deals damage ONLY when charging
    baseStats.speed = 2.5f; 
    
    fragmentDropCount = 1;
    
    setSteeringStrategy(std::make_unique<SeekStrategy>());
    changeState(std::make_unique<IdleState>());
}

void VoidShunter::updateState(float dt, Chamber& chamber) {
    float cellSize = SettingManager::getInstance().getCellSize();

    if (isCharging) {
        chargeTimer -= dt;
        
        // Move in charge direction
        const float SPEED_TO_PIXELS = cellSize * SettingManager::getInstance().getSpeedMultiplier();
        setVelocity(chargeDir * baseStats.speed * SPEED_TO_PIXELS);
        
        // Check collision with player
        sf::Vector2f pPos = getPlayer().getPosition();
        sf::Vector2f mPos = getPosition();
        if (Math::distance(pPos, mPos) < 40.0f) { // Contact distance
            getPlayer().takeDamage(11.0f); // Charge damage
            
            // Stop charging after hit
            isCharging = false;
            chargeCooldown = 3.0f;
            baseStats.speed = 2.5f;
            changeState(std::make_unique<IdleState>());
        } else if (chargeTimer <= 0.0f) {
            // Charge ended without hit
            isCharging = false;
            chargeCooldown = 3.0f;
            baseStats.speed = 2.5f;
            changeState(std::make_unique<IdleState>());
        }
        
        // Skip standard enemy update while charging
        return; 
    } else {
        chargeCooldown -= dt;
        
        sf::Vector2f pPos = getPlayer().getPosition();
        sf::Vector2f mPos = getPosition();
        float dist = Math::distance(pPos, mPos);
        
        // If cooldown is ready and player is at mid-range, start charging
        if (chargeCooldown <= 0.0f && dist < 8.0f * cellSize && dist > 3.0f * cellSize) {
            isCharging = true;
            chargeTimer = 1.5f; 
            baseStats.speed = 6.0f; // Speed 6.0 when charging
            chargeDir = Math::normalize(pPos - mPos);
            
            // Temporarily disable state logic so it strictly charges
            changeState(nullptr); 
        } else {
            // Normal behavior (with 0 base damage, so it won't hurt player in AttackingState)
            Enemy::updateState(dt, chamber);
        }
    }
}

void VoidShunter::onDeath(Chamber* chamber) {
    std::cout << "VoidShunter died!\n";
}
