#include "prevent-chamber.hpp"
#include "../entities/player.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../utils/math-utility.hpp"
#include "../entities/enemy/enemy-state.hpp"
#include <iostream>

PreventChamber::PreventChamber(Player& player, EchoType echoType)
    : Chamber(player), associatedEcho(echoType) {
    
    float cellSize = SettingManager::getInstance().getCellSize();
    exitShape.setSize(sf::Vector2f(cellSize, cellSize));
    exitShape.setOrigin(sf::Vector2f(cellSize / 2.0f, cellSize / 2.0f));
    exitShape.setFillColor(sf::Color(255, 100, 100, 150));
    exitShape.setOutlineColor(sf::Color::Red);
    exitShape.setOutlineThickness(2.0f);
}

void PreventChamber::setExitPosition(sf::Vector2f pos) {
    exitPosition = pos;
    exitShape.setPosition(pos);
}

void PreventChamber::update(float dt) {
    Chamber::update(dt);
    
    float cellSize = SettingManager::getInstance().getCellSize();
    for (auto& enemy : enemies) {
        // Check if carrier reached exit
        if (enemy->getIsRealCarrier()) {
            if (Math::distance(enemy->getPosition(), exitPosition) <= 0.5f * cellSize) {
                std::cout << "Carrier reached the exit! Echo STOLEN.\n";
                Game::getInstance().getRunState().echoOutcomes[associatedEcho] = EchoOutcome::STOLEN;
                enemy->takeDamage(9999.0f); // Kill the carrier so it stops triggering
                failChamber();
            }
        }
    }

    if (enemies.empty() && !isFailed) {
        completeChamber();
    }

    itemManager.update(dt, player, *this);
    checkCollisions(dt);
}

void PreventChamber::drawBackground(sf::RenderWindow& window) {
    // Draw exit zone
    window.draw(exitShape);
    
    for (auto it = debugHitboxes.begin(); it != debugHitboxes.end(); ) {
        CollisionSolver::drawDebug(window, it->shape);
        it->timer -= 0.016f;
        if (it->timer <= 0) {
            it = debugHitboxes.erase(it);
        } else {
            ++it;
        }
    }
}

void PreventChamber::onEnemyHit(Enemy* enemy, bool lethal) {
    // onCarrierHit(enemy, lethal);
    if (!lethal && enemy->getIsRealCarrier()) {
        // Trigger 0.5s stagger on real carrier
        enemy->changeState(std::make_unique<StaggeredState>(0.5f, std::make_unique<ChasingState>()));
        std::cout << "Real Carrier hit! Staggered.\n";
    }
}
