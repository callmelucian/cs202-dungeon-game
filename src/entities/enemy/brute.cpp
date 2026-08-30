#include "brute.hpp"
#include "enemy-state.hpp"
#include "enemy-steering-strategy.hpp"
#include "../player.hpp"
#include "../../ui/graphics/particle-system.hpp"
#include "../../ui/graphics/aura-renderer.hpp"
#include "../../global-settings/sound-manager.hpp"
#include "../../utils/math-utility.hpp"
#include <iostream>
#include <algorithm>

Brute::Brute(Player& player)
    : Enemy("brute", player),
      shootTimer(4.0f),
      shootInterval(4.0f),
      chargeDuration(0.8f) {
    baseStats.maxHp = 45.0f;
    baseStats.hp = 45.0f;
    baseStats.damage = 14.0f;
    baseStats.speed = 2.0f;

    setSteeringStrategy(std::make_unique<SeekStrategy>());
    changeState(std::make_unique<ChasingState>());
}

void Brute::update(float deltaTime) {
    Enemy::update(deltaTime);

    if (isAlive() && !isFrozen()) {
        shootTimer -= deltaTime;
        if (shootTimer <= 0.0f) {
            shootTimer = shootInterval;

            sf::Vector2f myPos = getPosition();
            sf::Vector2f targetPos = getPlayer().getPosition();
            sf::Vector2f dir = Math::normalize(targetPos - myPos);

            // Shoot 1 small orb aimed at Serin every 4s
            OrbProjectile orb(
                myPos,
                dir * 260.0f,
                10.0f,                                  // damage
                5.0f,                                   // visual radius
                12.0f,                                  // collision hitbox radius
                4.0f,                                   // lifetime
                sf::Color(240, 120, 30, 245),           // fiery amber core
                sf::Color(255, 210, 110, 255),          // bright gold outline
                1.5f
            );
            projectiles.push_back(orb);
            SoundManager::getInstance().playSound("fireball");
        }
    }

    // Update in-flight projectiles
    for (auto& proj : projectiles) {
        proj.update(deltaTime, getPlayer());
    }

    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(), [](const OrbProjectile& p) { return !p.isActive(); }),
        projectiles.end()
    );
}

void Brute::draw(sf::RenderWindow& window) const {
    // 1. Draw glowing telegraph charging aura prior to launching an orb
    if (isCharging()) {
        float chargeProgress = std::clamp(1.0f - (shootTimer / chargeDuration), 0.0f, 1.0f);
        float radius = 30.0f + chargeProgress * 16.0f;
        sf::Color coreColor(255, 140, 30, static_cast<uint8_t>(90 + chargeProgress * 90));
        sf::Color edgeColor(210, 50, 10, static_cast<uint8_t>(130 + chargeProgress * 110));
        AuraRenderer::getInstance().drawAura(window, getPosition(), radius, coreColor, edgeColor, 1.1f + chargeProgress * 0.4f, 2.0f);
    }

    Enemy::draw(window);

    for (const auto& proj : projectiles) {
        proj.draw(window);
    }
}

void Brute::onDeath(Chamber* chamber) {
    std::cout << "Brute destroyed! Emitting shockwave...\n";
    ParticleSystem::getInstance().emitBurst(getPosition(), 45, sf::Color(220, 100, 50, 220), 80.0f, 200.0f, 0.4f, 1.0f, 8.0f);
}
