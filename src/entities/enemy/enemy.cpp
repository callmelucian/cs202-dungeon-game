#include "enemy.hpp"
#include "enemy-state.hpp" 
#include "enemy-steering-strategy.hpp"
#include "../player.hpp"
#include "../../global-settings/sound-manager.hpp"
#include "../../global-settings/setting-manager.hpp"
#include "../../ui/graphics/particle-system.hpp"
#include "../../ui/graphics/aura-renderer.hpp"
#include <iostream>


Enemy::Enemy(const std::string& characterKey, Player& player)
    : Character(characterKey), 
      playerRef(player), 
      currentState(nullptr), 
      attackCooldown(0.0f),
      fragmentDropCount(1),
      isRealCarrier(false),
      hitWall(false) {
    setHealthBar(std::make_unique<UI::EnemyHealthBar>());
    if (healthBar) {
        healthBar->setHealth(getHp(), getEffectiveStats().maxHp, true);
    }
}

void Enemy::update(float deltaTime) {
    sf::Vector2f vel = getVelocity();
    
    if (std::abs(vel.x) > 0.01f || std::abs(vel.y) > 0.01f) {
        if (std::abs(vel.x) >= std::abs(vel.y)) {
            facingString = (vel.x > 0.f) ? "right" : "left";
            isFacingRight = (vel.x > 0.f);
        } else {
            facingString = (vel.y > 0.f) ? "down" : "up";
        }
        notifyStateChanged("walk-facing-" + facingString);
    } else {
        notifyStateChanged("idle-facing-" + facingString);
    }

    Character::update(deltaTime);
}

void Enemy::draw(sf::RenderWindow& window) const {
    if (isRealCarrier && isAlive()) {
        float radius = SettingManager::getInstance().getCellSize() * 0.55f;
        // Smooth radial gradient golden air aura radiating outward from the carrier
        sf::Color coreColor(255, 230, 110, 110); // Luminous golden core
        sf::Color edgeColor(240, 170, 30, 160);  // Warm amber air boundary
        AuraRenderer::getInstance().drawAura(window, getPosition(), radius, coreColor, edgeColor, 0.9f, 1.0f);
    }
    Character::draw(window);
}


void Enemy::takeDamage(float rawAmount, bool isCritical) {
    if (!isAlive()) return;
    Character::takeDamage(rawAmount, isCritical);
    SoundManager::getInstance().playSound("enemy-hit");
    
    if (!isAlive()) {
        ParticleSystem::getInstance().emitBurst(getPosition(), 30, sf::Color(150, 150, 150, 200), 50.0f, 150.0f, 0.3f, 0.8f, 6.0f);
    }
}

void Enemy::onWallCollision() {
    // Notify the steering strategy so it can invalidate its cached path and
    // replan around the obstacle on the next frame (fixes Bug 2: stale BFS path
    // repeatedly pressing the enemy into the same wall).
    if (steeringStrategy) {
        steeringStrategy->onWallHit();
    }

    // If knocked back heavily and hit a wall, award a bonus fragment.
    sf::Vector2f kv = getKnockbackVelocity();
    float kLen = kv.x * kv.x + kv.y * kv.y; // using squared length to avoid sqrt
    if (kLen > 100.0f * 100.0f) {
        if (!hitWall) {
            addBonusFragments(1);
            setHitWall(true);
            std::cout << "Enemy knocked into wall! +1 Bonus Fragment queued.\n";
        }
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