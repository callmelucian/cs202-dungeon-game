#include "enemy-state.hpp"
#include "enemy.hpp"
#include "enemy-steering-strategy.hpp"
#include "../player.hpp"
#include "../../utils/math-utility.hpp"
#include "../../global-settings/setting-manager.hpp"

void IdleState::onEnter(Enemy& enemy) {
    enemy.setVelocity(sf::Vector2f(0.0f, 0.0f));
}

void IdleState::onExit(Enemy& enemy) {}

void IdleState::update(Enemy& enemy, float dt, Chamber& chamber) {
    enemy.changeState(std::make_unique<ChasingState>());
}

void ChasingState::onEnter(Enemy& enemy) {}
void ChasingState::onExit(Enemy& enemy) {}

void ChasingState::update(Enemy& enemy, float dt, Chamber& chamber) {
    if (auto* strategy = enemy.getSteeringStrategy()) {
        sf::Vector2f desiredDirection = strategy->calculateSteering(enemy, enemy.getPlayer(), chamber);
        const float SPEED_TO_PIXELS = SettingManager::getInstance().getCellSize() * SettingManager::getInstance().getSpeedMultiplier();
        enemy.setVelocity(desiredDirection * enemy.getEffectiveStats().speed * SPEED_TO_PIXELS);
    }
    
    // Check range for attacking
    float dist = Math::distance(enemy.getPosition(), enemy.getPlayer().getPosition());
    if (dist < 40.0f) {
        enemy.changeState(std::make_unique<AttackingState>());
    }
}

void AttackingState::onEnter(Enemy& enemy) {
    enemy.setVelocity(sf::Vector2f(0.0f, 0.0f));
    attackTimer = 0.0f; // Reset timer on contact
}

void AttackingState::onExit(Enemy& enemy) {}

void AttackingState::update(Enemy& enemy, float dt, Chamber& chamber) {
    // Check if player moved out of contact range
    float dist = Math::distance(enemy.getPosition(), enemy.getPlayer().getPosition());
    if (dist >= 40.0f) {
        enemy.changeState(std::make_unique<ChasingState>());
        return;
    }
    
    attackTimer += dt;
    if (attackTimer >= SettingManager::getInstance().getEnemyAttackTime()) {
        enemy.getPlayer().takeDamage(enemy.getEffectiveStats().damage);
        attackTimer = 0.0f; // Reset timer after attacking
        
        // Optional: add a tiny stagger to show attack animation? Let's just keep attacking every 2s
        // enemy.changeState(std::make_unique<StaggeredState>(0.2f, std::make_unique<AttackingState>()));
    }
}

StaggeredState::StaggeredState(float duration, std::unique_ptr<EnemyState> previous) : duration(duration), elapsedTime(0.0f), previousState(std::move(previous)) {}

void StaggeredState::onEnter(Enemy& enemy) {
    enemy.setVelocity(sf::Vector2f(0.0f, 0.0f));
}

void StaggeredState::onExit(Enemy& enemy) {}

void StaggeredState::update(Enemy& enemy, float dt, Chamber& chamber) {
    elapsedTime += dt;
    if (elapsedTime >= duration) enemy.changeState(std::move(previousState));
}
