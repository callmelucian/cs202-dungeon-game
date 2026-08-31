#include "arrow-projectile.hpp"
#include "../../global-settings/asset-manager.hpp"
#include "../../ui/graphics/particle-system.hpp"
#include "../../ui/graphics/aura-renderer.hpp"
#include "../../utils/camera.hpp"
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
                                 ArrowHitMode hitMode,
                                 bool isRedLaser)
    : Projectile(startPos, direction, speed, maxDistance, 10.0f),
      sprite(AssetManager::getInstance().getTexture("arrow"), sf::IntRect({96, 64}, {32, 32})),
      hitMode(hitMode),
      isRedLaser(isRedLaser)
{
    sprite.setOrigin(sf::Vector2f(16.0f, 16.0f));

    float angleDeg = std::atan2(this->direction.y, this->direction.x) * 180.0f / static_cast<float>(M_PI);
    sprite.setRotation(sf::degrees(angleDeg + 45.0f));
    sprite.setPosition(position);

    if (isRedLaser) {
        sprite.setColor(sf::Color(255, 70, 70));
    }
}

void ArrowProjectile::update(float dt, Chamber& chamber, Player& player) {
    if (!active) return;

    sf::Vector2f prevPos = position;
    updateMotion(dt);
    sprite.setPosition(position);

    // Only red laser arrows emit trailing particles
    if (isRedLaser) {
        ParticleSystem::getInstance().emitSparkle(position, 1, sf::Color(255, 50, 50, 200), 10.0f);
    }

    // 1. Check collision with enemies along flight segment
    LineHitbox ray{prevPos, position};
    for (auto* enemy : chamber.getEnemiesRaw()) {
        if (!enemy || !enemy->isAlive()) continue;
        if (hitEnemies.count(enemy)) continue;

        if (CollisionSolver::checkCollision(ray, enemy->getBounds())) {
            hitEnemies.insert(enemy);

            float critRate = player.getCriticalHitRate();
            bool isCrit = false;
            if (!enemy->isShielded() && critRate > 0.0f) {
                float roll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                if (roll < critRate) {
                    isCrit = true;
                }
            }

            float damage = player.getEffectiveStats().damage;
            if (player.getStateMachine().getActiveState()) {
                damage = player.getStateMachine().getActiveState()->modifyOutgoingDamage(damage);
            }
            if (isCrit) {
                damage *= 2.0f;
            }

            enemy->takeDamage(damage, isCrit);

            if (isCrit) {
                AuraRenderer::getInstance().triggerScreenFlash(sf::Color(255, 255, 255, 200), 0.12f);
                Camera::triggerShake(7.0f, 0.15f);
                if (enemy->canBeKnockedBack()) {
                    sf::Vector2f dir = Math::normalize(enemy->getPosition() - player.getPosition());
                    enemy->applyKnockback(dir, 1600.0f);
                }
            }

            // Only red laser emits red particle burst on impact
            if (isRedLaser) {
                ParticleSystem::getInstance().emitBurst(enemy->getPosition(), 15, sf::Color(255, 50, 50, 240), 50.0f, 150.0f, 0.15f, 0.45f, 4.5f);
            }

            bool lethal = !enemy->isAlive();
            chamber.onEnemyHit(enemy, lethal);

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
                if (isRedLaser) {
                    ParticleSystem::getInstance().emitBurst(intersection, 6, sf::Color(255, 60, 60, 200), 20.0f, 60.0f, 0.1f, 0.2f, 3.0f);
                }
                break;
            }
        }
    }
}

void ArrowProjectile::draw(sf::RenderTarget& target) const {
    if (active) {
        target.draw(sprite);
    }
}

void ArrowProjectile::draw(sf::RenderWindow& window) const {
    if (active) {
        window.draw(sprite);
    }
}

ArrowHitMode ArrowProjectile::getHitMode() const {
    return hitMode;
}
