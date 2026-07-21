#include "mid-chamber.hpp"
#include "../global-settings/setting-manager.hpp"
#include <iostream>

MidChamber::MidChamber(Player& player) : Chamber(player) {
    // The grid will be loaded externally via ChamberFactory and MapLoader
}

void MidChamber::update(float dt) {
    for (auto it = enemies.begin(); it != enemies.end(); ) {
        if (!(*it)->isAlive()) {
            (*it)->onDeath();
            it = enemies.erase(it);
        } else {
            (*it)->update(dt);
            (*it)->updateState(dt, *this);
            ++it;
        }
    }
    
    checkCollisions(dt);
    
    for (auto it = debugHitboxes.begin(); it != debugHitboxes.end(); ) {
        it->timer -= dt;
        if (it->timer <= 0) {
            it = debugHitboxes.erase(it);
        } else {
            ++it;
        }
    }
}

void MidChamber::draw(sf::RenderWindow& window) {
    // Draw grid tiles
    window.draw(tileMap);

    for (const auto& enemy : enemies) {
        enemy->draw(window);
    }

    for (const auto& hb : debugHitboxes) {
        CollisionSolver::drawDebug(window, hb.shape);
    }
}

