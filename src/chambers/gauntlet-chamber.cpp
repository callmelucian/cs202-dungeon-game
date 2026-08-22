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

    if (waveSpawner.isFinished() && enemies.empty() && !isCompleted) {
        float healAmount = player.getEffectiveStats().maxHp * 0.25f;
        player.heal(healAmount);
        std::cout << "GauntletChamber: Chamber cleared! Granted +25% MaxHP heal (" << healAmount << " HP).\n";
        completeChamber();
    }
}
