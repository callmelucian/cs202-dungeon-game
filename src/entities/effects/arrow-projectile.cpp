#include "arrow-projectile.hpp"
#include "../../global-settings/asset-manager.hpp"
#include "../../graphics/particle-system.hpp"
#include "../../utils/math-utility.hpp"
#include "../../utils/collision-solver.hpp"
#include "../../chambers/chamber.hpp"
#include "../player.hpp"
#include "../enemy/enemy.hpp"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ArrowProjectile::ArrowProjectile(sf::Vector2f startPos,
                                 sf::Vector2f direction,
                                 float maxDistance,
                                 float speed,
                                 ArrowHitMode hitMode)
    : sprite(AssetManager::getInstance().getTexture("arrow"), sf::IntRect({96, 64}, {32, 32})),
      startPosition(startPos),
      position(startPos),
      direction(Math::normalize(direction)),
      speed(speed),
      distanceTraveled(0.0f),
      maxDistance(maxDistance),
      hitMode(hitMode),
      active(true)
{
    sprite.setOrigin(sf::Vector2f(16.0f, 16.0f));

    float angleDeg = std::atan2(this->direction.y, this->direction.x) * 180.0f / static_cast<float>(M_PI);
    sprite.setRotation(sf::degrees(angleDeg + 45.0f));
    sprite.setPosition(position);
}

void ArrowProjectile::update(float dt, Chamber& chamber, Player& player) {
    if (!active) return;

    sf::Vector2f prevPos = position;
    float step = speed * dt;
    position += direction * step;
    distanceTraveled += step;
    sprite.setPosition(position);

    // Emit subtle void sparkle trail
    ParticleSystem::getInstance().emitSparkle(position, 1, sf::Color(180, 80, 255, 180), 8.0f);

    // 1. Check collision with enemies along flight segment
    LineHitbox ray{prevPos, position};
    for (auto* enemy : chamber.getEnemiesRaw()) {
        if (!enemy || !enemy->isAlive()) continue;
        if (hitEnemies.count(enemy)) continue;

        if (CollisionSolver::checkCollision(ray, enemy->getBounds())) {
            hitEnemies.insert(enemy);

            float damage = player.getEffectiveStats().damage;
            if (player.getStateMachine().getActiveState()) {
                damage = player.getStateMachine().getActiveState()->modifyOutgoingDamage(damage);
            }

            enemy->takeDamage(damage);
            bool lethal = !enemy->isAlive();
            chamber.onEnemyHit(enemy, lethal);

            // Emit void purple impact particles at enemy position
            ParticleSystem::getInstance().emitBurst(enemy->getPosition(), 10, sf::Color(180, 80, 255, 220), 40.0f, 120.0f, 0.1f, 0.3f, 4.0f);

            // Momentum gain
            float baseGain = (distanceTraveled >= 6.0f * 60.0f) ? 8.0f : 4.0f;
            player.gainMomentum(baseGain, FormType::VOIDCASTER);

            if (hitMode == ArrowHitMode::SINGLE_TARGET) {
                active = false;
                break;
            }
        }
    }

    // 2. Check collision with solid wall obstacles (elevation height logic)
    if (active) {
        auto obstacles = chamber.getArrowSolidObstacles(startPosition);
        for (const auto& obs : obstacles) {
            sf::Vector2f intersection;
            if (CollisionSolver::lineIntersectsRect(prevPos, position, obs, intersection)) {
                active = false;
                ParticleSystem::getInstance().emitBurst(intersection, 6, sf::Color(200, 200, 200, 200), 20.0f, 60.0f, 0.1f, 0.2f, 3.0f);
                break;
            }
        }
    }

    // 3. Max distance check
    if (distanceTraveled >= maxDistance) {
        active = false;
    }
}

void ArrowProjectile::draw(sf::RenderWindow& window) const {
    if (active) {
        window.draw(sprite);
    }
}

bool ArrowProjectile::isActive() const {
    return active;
}

void ArrowProjectile::deactivate() {
    active = false;
}

sf::Vector2f ArrowProjectile::getPosition() const {
    return position;
}

sf::Vector2f ArrowProjectile::getDirection() const {
    return direction;
}

float ArrowProjectile::getSpeed() const {
    return speed;
}

ArrowHitMode ArrowProjectile::getHitMode() const {
    return hitMode;
}
