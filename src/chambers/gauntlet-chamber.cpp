#include "gauntlet-chamber.hpp"
#include "../entities/player.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../utils/math-utility.hpp"
#include "../core/game.hpp"
#include <iostream>

GauntletChamber::GauntletChamber(Player& player) : Chamber(player) {
    dropsFragments = false;
}

void GauntletChamber::update(float dt) {
    Chamber::update(dt);

    if (isHungerPit) {
        float cell = SettingManager::getInstance().getCellSize();
        float ox = SettingManager::getInstance().getGridOffsetX();
        float oy = SettingManager::getInstance().getGridOffsetY();
        
        const auto& grid = getTypeGrid();
        float centerX = ox + (grid.empty() ? 10.0f : static_cast<float>(grid[0].size()) / 2.0f) * cell;
        float centerY = oy + (grid.empty() ? 10.0f : static_cast<float>(grid.size()) / 2.0f) * cell;
        sf::Vector2f centerPos(centerX, centerY);

        float voidRadius = 5.0f * cell;

        // Check player contact with central void hazard
        if (player.isAlive()) {
            float dist = Math::distance(player.getPosition(), centerPos);
            if (dist <= voidRadius) {
                player.takeDamage(9999.0f);
                std::cout << "Hunger Pit Void! Player fell into the central void hazard.\n";
            }
        }

        // Check enemy contact with central void hazard
        for (auto* enemy : getEnemiesRaw()) {
            if (enemy && enemy->isAlive()) {
                float dist = Math::distance(enemy->getPosition(), centerPos);
                if (dist <= voidRadius) {
                    enemy->takeDamage(9999.0f);
                    std::cout << "Hunger Pit Void! Enemy pushed into central void hazard.\n";
                }
            }
        }
    }

    if (waveSpawner.isFinished() && enemies.empty() && !isCompleted) {
        float healAmount = player.getEffectiveStats().maxHp * 0.25f;
        player.heal(healAmount);
        std::cout << "GauntletChamber: Chamber cleared! Granted +25% MaxHP heal (" << healAmount << " HP).\n";
        completeChamber();
    }
}

void GauntletChamber::drawForeground(sf::RenderWindow& window) {
    Chamber::drawForeground(window);

    if (isHungerPit) {
        float cell = SettingManager::getInstance().getCellSize();
        float ox = SettingManager::getInstance().getGridOffsetX();
        float oy = SettingManager::getInstance().getGridOffsetY();

        const auto& grid = getTypeGrid();
        float centerX = ox + (grid.empty() ? 10.0f : static_cast<float>(grid[0].size()) / 2.0f) * cell;
        float centerY = oy + (grid.empty() ? 10.0f : static_cast<float>(grid.size()) / 2.0f) * cell;

        float voidRadius = 5.0f * cell;
        sf::CircleShape pit(voidRadius);
        pit.setOrigin({voidRadius, voidRadius});
        pit.setPosition({centerX, centerY});
        pit.setFillColor(sf::Color(10, 5, 20, 230));

        // Task B6 check: reduce pit edge warning glow opacity by 60% if Obsidian Key was stolen!
        auto keyIt = Game::getInstance().getRunState().echoOutcomes.find(EchoType::OBSIDIAN_KEY);
        bool obsidianStolen = (keyIt != Game::getInstance().getRunState().echoOutcomes.end() && keyIt->second == EchoOutcome::STOLEN);
        uint8_t outlineAlpha = obsidianStolen ? static_cast<uint8_t>(220 * 0.4f) : 220;

        pit.setOutlineColor(sf::Color(255, 60, 60, outlineAlpha));
        pit.setOutlineThickness(4.0f);

        window.draw(pit);
    }
}
