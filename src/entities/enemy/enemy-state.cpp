#include "enemy-state.hpp"
#include "enemy.hpp"
#include "enemy-steering-strategy.hpp"
#include "../player.hpp"

void IdleState::onEnter(Enemy& enemy) {
    enemy.setVelocity(sf::Vector2f(0.0f, 0.0f));
}

void IdleState::onExit(Enemy& enemy) {}

void IdleState::update(Enemy& enemy, float dt, Chamber& chamber) {
    // Default: enemies will immediately wake up and start chasing
    enemy.changeState(new ChasingState());
}

void ChasingState::onEnter(Enemy& enemy) {}
void ChasingState::onExit(Enemy& enemy) {}

void ChasingState::update(Enemy& enemy, float dt, Chamber& chamber) {
    if (auto* strategy = enemy.getSteeringStrategy()) {
        sf::Vector2f desiredDirection = strategy->calculateSteering(enemy, enemy.getPlayer(), chamber);
        enemy.setVelocity(desiredDirection * enemy.getEffectiveStats().speed);
    }
}

void AttackingState::onEnter(Enemy& enemy) {
    enemy.setVelocity(sf::Vector2f(0.0f, 0.0f));
}

void AttackingState::onExit(Enemy& enemy) {}

void AttackingState::update(Enemy& enemy, float dt, Chamber& chamber) {
    // TODO: Implement
    // Waiting for other features to be implemented (like attacking animation)
}

// ================= STAGGERED STATE =================
StaggeredState::StaggeredState(float duration, EnemyState* previous) : duration(duration), elapsedTime(0.0f), previousState(previous) {}

void StaggeredState::onEnter(Enemy& enemy) {
    enemy.setVelocity(sf::Vector2f(0.0f, 0.0f));
}

void StaggeredState::onExit(Enemy& enemy) {}

void StaggeredState::update(Enemy& enemy, float dt, Chamber& chamber) {
    elapsedTime += dt;
    if (elapsedTime >= duration) enemy.changeState(previousState);
}
