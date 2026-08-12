#include "gauntlet-chamber.hpp"
#include "../entities/player.hpp"
#include <iostream>

GauntletChamber::GauntletChamber(Player& player) : Chamber(player) {
    dropsFragments = false;
}

void GauntletChamber::setWaves(const std::vector<WaveConfig>& configs) {
    waveSpawner.setWaves(configs);
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
