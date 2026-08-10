#include "protect-chamber.hpp"
#include "../entities/player.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../utils/math-utility.hpp"
#include "../utils/pathfinder.hpp"
#include "../global-settings/sound-manager.hpp"
#include "../graphics/particle-system.hpp"
#include <iostream>

ProtectChamber::ProtectChamber(Player& player, const std::string& echoName, float requiredTime)
    : Chamber(player), collectionTimer(0.0f), requiredCollectionTime(requiredTime), isCollected(false) {
    echo = std::make_unique<Echo>(echoName, 50.0f); // Starts at 50% power

    echoShape.setRadius(20.0f);
    echoShape.setOrigin(sf::Vector2f(20.0f, 20.0f));
    echoShape.setFillColor(sf::Color::Cyan);

    float cellSize = SettingManager::getInstance().getCellSize();
    float radiusPx = 2.5f * cellSize;
    radiusShape.setRadius(radiusPx);
    radiusShape.setOrigin(sf::Vector2f(radiusPx, radiusPx));
    radiusShape.setFillColor(sf::Color(0, 255, 255, 30));
    radiusShape.setOutlineColor(sf::Color::Cyan);
    radiusShape.setOutlineThickness(2.0f);
}

void ProtectChamber::setEchoPosition(sf::Vector2f pos) {
    echoPosition = pos;
    echoShape.setPosition(pos);
    radiusShape.setPosition(pos);
    // Prime the bar's sprite positions immediately so they're correct on the
    // first draw (during the chamber intro, before update() has ever run).
    collectorTimerBar.setPosition(echoPosition + sf::Vector2f(-29.0f, -35.0f));
}

void ProtectChamber::update(float dt) {
    Chamber::update(dt);

    // 2. Update Echo collection
    if (!isCollected) {
        float cellSize = SettingManager::getInstance().getCellSize();
        float distToPlayer = Math::distance(player.getPosition(), echoPosition);
        
        if (distToPlayer <= 2.5f * cellSize) {
            collectionTimer += dt;
            if (collectionTimer >= requiredCollectionTime) {
                isCollected = true;
                SoundManager::getInstance().playSound("echo-collect");
                ParticleSystem::getInstance().emitGlow(echoPosition, 40, sf::Color(255, 255, 200, 200), 50.0f);
                std::cout << "Echo Collected! Final Power: " << echo->getPower() << "%\n";
                completeChamber();
            }
        }

        // 3. Check enemy attacks on Echo
        for (Enemy* enemy : getEnemiesRaw()) {
            if (!enemy || !enemy->isAlive()) continue;
            float distToEcho = Math::distance(enemy->getPosition(), echoPosition);
            if (distToEcho <= 1.5f * cellSize) {
                if (enemy->getAttackCooldown() <= 0.0f) {
                    onEchoHit(enemy->getEffectiveStats().damage);
                    enemy->setAttackCooldown(2.0f);
                }
            }
        }

        // Update Echo collection timer bar (EnemyHealthBar UI)
        collectorTimerBar.setHealth(collectionTimer, requiredCollectionTime);
        collectorTimerBar.setPosition(echoPosition + sf::Vector2f(-29.0f, -35.0f));
        collectorTimerBar.update(dt);
    }
}

void ProtectChamber::drawBackground(sf::RenderWindow& window) {
    if (!isCollected) {
        window.draw(radiusShape);
        window.draw(echoShape);
    }
}

void ProtectChamber::drawForeground(sf::RenderWindow& window) {
    if (!isCollected) {
        collectorTimerBar.draw(window);
    }
}

void ProtectChamber::onEnemyHit(Enemy* enemy, bool lethal) {
    Chamber::onEnemyHit(enemy, lethal);
}

void ProtectChamber::onEchoHit(float rawDamage) {
    if (!echo || isCollected) return;
    if (checkIronshellRedirect()) {
        player.takeDamage(rawDamage);
        std::cout << "Ironshell absorbed Echo hit! Damage redirected to Serin.\n";
    } else {
        // 8% penalty per hit on Echo
        echo->takeDamage(8.0f);
        std::cout << "Echo hit! Power reduced to " << echo->getPower() << "%\n";
    }
}

void ProtectChamber::onFragmentCollected(float value) {
    if (isCollected) return; // Echo is already fully stabilized
    
    // Mid-collection is when the capture progress has started (timer > 0)
    bool midCollection = (collectionTimer > 0.0f);
    
    // Formula: +5.0% pre-collection, +2.5% mid-collection per fragment unit
    float powerGain = midCollection ? (value * 2.5f) : (value * 5.0f);
    
    if (echo) {
        echo->addPower(powerGain);
        std::cout << "ProtectChamber: Echo Power increased by " << powerGain << "%. New Power: " << echo->getPower() << "%\n";
    }
}

bool ProtectChamber::checkIronshellRedirect() {
    if (player.getActiveFormType() == FormType::IRONSHELL) {
        float cellSize = SettingManager::getInstance().getCellSize();
        float dist = Math::distance(player.getPosition(), echoPosition);
        if (dist <= 1.0f * cellSize) {
            return true;
        }
    }
    return false;
}