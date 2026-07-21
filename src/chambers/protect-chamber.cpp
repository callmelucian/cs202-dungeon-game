#include "protect-chamber.hpp"
#include "../entities/player.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../utils/math-utility.hpp"
#include "../utils/pathfinder.hpp"
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
                std::cout << "Echo Collected! Final Power: " << echo->getPower() << "%\n";
                completeChamber();
            }
        }
    }
}

void ProtectChamber::drawBackground(sf::RenderWindow& window) {
    if (!isCollected) {
        window.draw(radiusShape);
        window.draw(echoShape);
    }
}

void ProtectChamber::onEnemyHit(Enemy* enemy, bool lethal) {
    // Apply Wraithblade knockback if active form is Wraithblade
    if (player.getActiveFormType() == FormType::WRAITHBLADE) {
        applyWraithbladeKnockback(enemy);
    }
}

void ProtectChamber::onEchoHit(float rawDamage) {
    // 8% penalty per hit on Echo
    echo->takeDamage(8.0f);
    std::cout << "Echo hit! Power reduced to " << echo->getPower() << "%\n";
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

void ProtectChamber::applyWraithbladeKnockback(Enemy* enemy) {
    float cellSize = SettingManager::getInstance().getCellSize();
    sf::Vector2f dir = Math::normalize(enemy->getPosition() - echoPosition);
    // Push enemy 4 units away from Echo
    sf::Vector2f targetPos = enemy->getPosition() + dir * (4.0f * cellSize);
    
    // Move enemy safely
    if (Pathfinder::isWalkable(targetPos, getGrid())) {
        enemy->setPosition(targetPos);
    } else {
        // Collided with wall/obstacle! Grant +1 bonus fragment if not already awarded
        if (enemy && !enemy->getHitWall()) {
            enemy->addBonusFragments(1);
            enemy->setHitWall(true);
            std::cout << "Enemy knocked into wall! +1 Bonus Fragment queued.\n";
        }
    }
}