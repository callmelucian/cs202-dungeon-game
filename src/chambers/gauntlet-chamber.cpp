#include "gauntlet-chamber.hpp"
#include "../entities/player.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../utils/math-utility.hpp"
#include "../core/game.hpp"
#include <iostream>

#include "../ui/widgets/floating-text-manager.hpp"

GauntletChamber::GauntletChamber(Player& player) : Chamber(player) {
    dropsFragments = false;
}

void GauntletChamber::update(float dt) {
    Chamber::update(dt);

    if (waveSpawner.isFinished() && enemies.empty() && !isCompleted && !cleared) {
        cleared = true;
        float healAmount = player.getEffectiveStats().maxHp * 0.25f;
        player.heal(healAmount);
        std::cout << "GauntletChamber: Chamber cleared! Granted +25% MaxHP heal (" << healAmount << " HP).\n";
        
        if (exitGate) {
            exitGate->setActive(true);
        }
        sf::Vector2f gateNotifyPos = (exitPosition.x >= 0.0f) ? (exitPosition + sf::Vector2f(0.0f, -40.0f)) : (player.getPosition() + sf::Vector2f(0.0f, -30.0f));
        UI::FloatingTextManager::getInstance().spawnText(gateNotifyPos, "GAUNTLET CLEARED - ENTER EXIT GATE", sf::Color(100, 255, 200), 12, "header", 3.0f);
    }
}
