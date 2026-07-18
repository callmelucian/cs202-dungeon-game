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
}

void AttackingState::onExit(Enemy& enemy) {}

void AttackingState::update(Enemy& enemy, float dt, Chamber& chamber) {
    // Deal damage and return to chasing
    enemy.getPlayer().takeDamage(enemy.getEffectiveStats().damage);
    // Add a simple stagger or just switch back to chasing (Enemy class will handle cooldown if we had one)
    // For now just transition back so it doesn't spam attack every frame.
    // Actually we should use StaggeredState to act as a cooldown.
    enemy.changeState(std::make_unique<StaggeredState>(1.0f, std::make_unique<ChasingState>()));
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
