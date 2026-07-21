#include "mid-chamber.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../entities/player.hpp"
#include <iostream>

MidChamber::MidChamber(Player& player) : Chamber(player) {
    // onEnter
    player.setInMidChamber(true);
}

MidChamber::~MidChamber() {
    // Failsafe in case state transitions destroy it before completeChamber finishes
    if (player.getInMidChamber()) {
        player.setInMidChamber(false);
    }
}

void MidChamber::completeChamber() {
    if (!isCompleted && !isFailed) {
        player.setInMidChamber(false);
        player.gainMomentum(15.0f, player.getActiveFormType());
    }
    Chamber::completeChamber();
}

void MidChamber::update(float dt) {
    Chamber::update(dt);
    
    // For prototype, automatically complete if there's no custom exit logic,
    // or we can wait for player to reach an exit point. Since there's no exit point
    // defined, let's complete if enemies are empty (assuming a test or just instant).
    if (enemies.empty()) {
        completeChamber();
    }
}

