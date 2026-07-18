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
    float cellSize = SettingManager::getInstance().getCellSize();
    
    for (auto it = enemies.begin(); it != enemies.end(); ) {
        if (!(*it)->isAlive()) {
            (*it)->onDeath();
            
            // Prevent chambers have no collection timer, full 5% per frag.
            // (handled by standard item drop / echo power logic if we tied them together,
            // but for prevent chamber, the echo is stolen or not. So no echo power modification here)

            it = enemies.erase(it);
        } else {
            (*it)->update(dt);
            (*it)->updateState(dt, *this);
            
            // Check if carrier reached exit
            if ((*it)->getIsRealCarrier()) {
                if (Math::distance((*it)->getPosition(), exitPosition) <= 0.5f * cellSize) {
                    std::cout << "Carrier reached the exit! Echo STOLEN.\n";
                    Game::getInstance().getRunState().echoOutcomes[associatedEcho] = EchoOutcome::STOLEN;
                    (*it)->takeDamage(9999.0f); // Kill the carrier so it stops triggering
                }
            }
            
            ++it;
        }
    }

    checkCollisions(dt);
}

void PreventChamber::draw(sf::RenderWindow& window) {
    window.draw(tileMap);
    
    // Draw exit zone
    window.draw(exitShape);

    for (const auto& item : items) {
        window.draw(*item);
    }
    for (const auto& enemy : enemies) {
        enemy->draw(window);
    }
    
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

void PreventChamber::processPlayerAttack(const Hitbox& hitbox) {
    debugHitboxes.push_back({hitbox, 0.2f});
    for (auto& enemy : enemies) {
        if (!enemy->isAlive()) continue;

        if (CollisionSolver::checkCollision(hitbox, enemy->getBounds())) {
            float damage = player.getEffectiveStats().damage;
            bool lethal = (enemy->getHp() - damage) <= 0;
            
            enemy->takeDamage(damage);
            
            onCarrierHit(enemy.get(), lethal);
        }
    }
}

void PreventChamber::onCarrierHit(Enemy* enemy, bool lethal) {
    if (!lethal && enemy->getIsRealCarrier()) {
        // Trigger 0.5s stagger on real carrier
        enemy->changeState(std::make_unique<StaggeredState>(0.5f, std::make_unique<ChasingState>()));
        std::cout << "Real Carrier hit! Staggered.\n";
    }
}
