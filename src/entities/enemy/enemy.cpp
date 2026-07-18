#include "enemy.hpp"
#include "enemy-state.hpp" 
#include "enemy-steering-strategy.hpp"
#include "../player.hpp"

Enemy::Enemy(const std::string& characterKey, Player& player)
    : Character(characterKey), 
      playerRef(player), 
      currentState(nullptr), 
      attackCooldown(0.0f),
      fragmentDropCount(1),
      isFacingRight(true) {
}

void Enemy::update(float deltaTime) {
    sf::Vector2f vel = getVelocity();
    setPosition(getPosition() + vel * deltaTime);
    
    if (vel.x < 0) isFacingRight = false;
    else if (vel.x > 0) isFacingRight = true;
    
    std::string direction = isFacingRight ? "right" : "left";
    if (vel.x != 0.f || vel.y != 0.f) {
        notifyStateChanged("walk-facing-" + direction);
    } else {
        notifyStateChanged("idle-facing-" + direction);
    }
    
    Character::update(deltaTime);
}

void Enemy::updateState(float dt, Chamber& chamber) {
    if (attackCooldown > 0.0f) attackCooldown -= dt;
    if (currentState) currentState->update(*this, dt, chamber);
}

void Enemy::changeState(std::unique_ptr<EnemyState> newState) {
    if (currentState) currentState->onExit(*this);
    currentState = std::move(newState);
    if (currentState) currentState->onEnter(*this);
}

void Enemy::setSteeringStrategy(std::unique_ptr<EnemySteeringStrategy> strategy) {
    steeringStrategy = std::move(strategy);
}

Player& Enemy::getPlayer() const {
    return playerRef;
}

EnemySteeringStrategy* Enemy::getSteeringStrategy() const {
    return steeringStrategy.get();
}

Enemy::~Enemy() = default;