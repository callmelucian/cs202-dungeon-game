#include "hushed-stalker.hpp"
#include "enemy-state.hpp"
#include "enemy-steering-strategy.hpp"
#include "../../chambers/chamber.hpp"
#include "../player.hpp"
#include "../animation/character-animator.hpp"
#include "../../global-settings/setting-manager.hpp"
#include <iostream>
#include <cmath>

HushedStalker::HushedStalker(Player& player) 
    : Enemy("hushed_stalker", player), windupTimer(0.0f), windingUp(false), isVisible(false) {
    
    baseStats.hp = 22.0f;
    baseStats.maxHp = 22.0f;
    baseStats.damage = 9.0f;
    baseStats.speed = 5.0f;
    
    fragmentDropCount = 1;
    attackCooldown = 0.0f;

    setSteeringStrategy(std::make_unique<SeekStrategy>());
    changeState(std::make_unique<IdleState>());
}

void HushedStalker::updateState(float dt, Chamber& chamber) {
    isVisible = isSlowed() || windingUp;

    if (windingUp) {
        windupTimer -= dt;
        if (windupTimer <= 0.0f) {
            windingUp = false;
            changeState(std::make_unique<AttackingState>());
            attackCooldown = 1.0f;
        }
        return;
    }

    // Determine if we should wind up to attack
    sf::Vector2f myPos = getPosition();
    sf::Vector2f playerPos = getPlayer().getPosition();
    float distToPlayer = std::hypot(myPos.x - playerPos.x, myPos.y - playerPos.y);
    float cellSize = SettingManager::getInstance().getCellSize();

    if (distToPlayer < 2.5f * cellSize && attackCooldown <= 0.0f) {
        windingUp = true;
        windupTimer = 0.4f;
        setVelocity(sf::Vector2f(0.0f, 0.0f));
        isVisible = true;
        return;
    }

    Enemy::updateState(dt, chamber);
}

void HushedStalker::draw(sf::RenderWindow& window) const {
    if (animator) {
        if (isVisible) {
            animator->setTint(sf::Color::White);
        } else {
            // Stealth mode: cloaked semi-transparent shimmer
            animator->setTint(sf::Color(120, 160, 255, 45));
        }
    }
    Character::draw(window);
}

void HushedStalker::onDeath(Chamber* chamber) {
    if (isSlowed()) fragmentDropCount = 2;
    else fragmentDropCount = 1;
}
