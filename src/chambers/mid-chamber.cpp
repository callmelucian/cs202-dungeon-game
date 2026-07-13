#include "mid-chamber.hpp"
#include "../global-settings/setting-manager.hpp"
#include <iostream>

MidChamber::MidChamber(Player& player) : Chamber(player) {
    // The grid will be loaded externally via ChamberFactory and MapLoader
}

void MidChamber::update(float dt) {
    for (auto& enemy : enemies) {
        enemy->update(dt);
        enemy->updateState(dt, *this);
    }
    
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
    float size = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();
    for (size_t y = 0; y < grid.size(); ++y) {
        for (size_t x = 0; x < grid[y].size(); ++x) {
            sf::RectangleShape cell({size, size});
            cell.setPosition({ox + static_cast<float>(x) * size, oy + static_cast<float>(y) * size});
            
            int tileType = grid[y][x];
            if (tileType == 1) {
                cell.setFillColor(sf::Color(47, 79, 79)); // Dark Slate Gray (Outer Wall)
            } else if (tileType == 2) {
                cell.setFillColor(sf::Color(180, 50, 50)); // Red (Inner Obstacle)
            } else if (tileType == 0) {
                cell.setFillColor(sf::Color(30, 30, 30)); // Dark Gray (Floor)
            }
            window.draw(cell);
        }
    }

    for (const auto& enemy : enemies) {
        enemy->draw(window);
    }

    for (const auto& hb : debugHitboxes) {
        CollisionSolver::drawDebug(window, hb.shape);
    }
}

void MidChamber::processPlayerAttack(const Hitbox& hitbox) {
    debugHitboxes.push_back({hitbox, 0.2f});
    // In the future, check hit against enemies
}
