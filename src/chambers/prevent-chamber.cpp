#include "prevent-chamber.hpp"
#include "../entities/player.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../global-settings/sound-manager.hpp"
#include "../graphics/particle-system.hpp"
#include "../utils/math-utility.hpp"
#include "../entities/enemy/enemy-state.hpp"
#include "../core/game.hpp"
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
        if (enemy->getIsRealCarrier() && enemy->isAlive()) {
            if (Math::distance(enemy->getPosition(), exitPosition) <= 0.5f * cellSize) {
                std::cout << "Carrier reached the exit! Echo STOLEN.\n";
                SoundManager::getInstance().playSound("enemy-hit");
                ParticleSystem::getInstance().emitBurst(exitPosition, 30, sf::Color(255, 50, 50, 200), 40.0f, 120.0f, 0.3f, 0.8f, 5.0f);
                RunState& runState = Game::getInstance().getRunState();
                runState.echoOutcomes[associatedEcho] = EchoOutcome::STOLEN;
                runState.echoesStolen++;
                
                // Carrier escapes with the Echo — drops 0 fragments on exit gate
                enemy->addBonusFragments(-enemy->getFragmentDropCount());
                enemy->setHp(0.0f); // Escaped carrier removed, chamber continues!
            }
        }
    }

    if (waveSpawner.isFinished() && enemies.empty() && !isFailed) {
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
    Chamber::onEnemyHit(enemy, lethal);
    // onCarrierHit(enemy, lethal);
    if (!lethal && enemy->getIsRealCarrier()) {
        // Trigger 0.5s stagger on real carrier
        enemy->changeState(std::make_unique<StaggeredState>(0.5f, std::make_unique<ChasingState>()));
        std::cout << "Real Carrier hit! Staggered.\n";
    }
}
