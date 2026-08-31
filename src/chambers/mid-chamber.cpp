#include "mid-chamber.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../entities/player.hpp"
#include "../utils/math-utility.hpp"
#include <iostream>

MidChamber::MidChamber(Player& player) : Chamber(player) {
    player.setInMidChamber(true);
    if (exitGate) {
        exitGate->setActive(true);
    }
}

MidChamber::~MidChamber() {
    // Failsafe in case state transitions destroy it before completeChamber finishes
    if (player.getInMidChamber()) {
        player.setInMidChamber(false);
    }
}

void MidChamber::setExitPosition(const sf::Vector2f& pos) {
    Chamber::setExitPosition(pos);
    if (exitGate) {
        exitGate->setActive(true);
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
}
