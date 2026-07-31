#include "enemy.hpp"
#include "enemy-state.hpp" 
#include "enemy-steering-strategy.hpp"
#include "../player.hpp"
#include "../../global-settings/sound-manager.hpp"
#include "../../graphics/particle-system.hpp"

Enemy::Enemy(const std::string& characterKey, Player& player)
    : Character(characterKey), 
      playerRef(player), 
      currentState(nullptr), 
      attackCooldown(0.0f),
      fragmentDropCount(1),
      isFacingRight(true),
      isRealCarrier(false),
      hitWall(false) {
    setHealthBar(std::make_unique<UI::EnemyHealthBar>());
    if (healthBar) {
        healthBar->setHealth(getHp(), getEffectiveStats().maxHp, true);
    }
}

void Enemy::update(float deltaTime) {
    sf::Vector2f vel = getVelocity();
    // Position integration is handled by CollisionSolver in Chamber::checkCollisions()
    
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

void Enemy::takeDamage(float rawAmount) {
    if (!isAlive()) return;
    Character::takeDamage(rawAmount);
    SoundManager::getInstance().playSound("enemy-hit");
    
    if (!isAlive()) {
        ParticleSystem::getInstance().emitBurst(getPosition(), 30, sf::Color(150, 150, 150, 200), 50.0f, 150.0f, 0.3f, 0.8f, 6.0f);
    }
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

void Enemy::setIsRealCarrier(bool real) {
    isRealCarrier = real;
}

bool Enemy::getIsRealCarrier() const {
    return isRealCarrier;
}

EnemySteeringStrategy* Enemy::getSteeringStrategy() const {
    return steeringStrategy.get();
}

void Enemy::addBonusFragments(int count) {
    fragmentDropCount += count;
}

int Enemy::getFragmentDropCount() const {
    return fragmentDropCount;
}

void Enemy::setHitWall(bool hit) {
    hitWall = hit;
}

bool Enemy::getHitWall() const {
    return hitWall;
}

UI::EnemyHealthBar* Enemy::getEnemyHealthBar() {
    return dynamic_cast<UI::EnemyHealthBar*>(healthBar.get());
}

const UI::EnemyHealthBar* Enemy::getEnemyHealthBar() const {
    return dynamic_cast<const UI::EnemyHealthBar*>(healthBar.get());
}

Enemy::~Enemy() = default;