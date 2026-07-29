#include "mid-chamber.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../entities/player.hpp"
#include "../utils/math-utility.hpp"
#include <iostream>

MidChamber::MidChamber(Player& player) : Chamber(player) {
    player.setInMidChamber(true);

    float cellSize = SettingManager::getInstance().getCellSize();

    // Exit trigger zone — a 1-cell square drawn as a glowing doorway.
    exitShape.setSize(sf::Vector2f(cellSize, cellSize));
    exitShape.setOrigin(sf::Vector2f(cellSize / 2.f, cellSize / 2.f));
    exitShape.setFillColor(sf::Color(200, 255, 200, 60));
    exitShape.setOutlineColor(sf::Color(100, 255, 100, 220));
    exitShape.setOutlineThickness(3.f);
}

MidChamber::~MidChamber() {
    // Failsafe in case state transitions destroy it before completeChamber finishes
    if (player.getInMidChamber()) {
        player.setInMidChamber(false);
    }
}

void MidChamber::setExitPosition(sf::Vector2f pos) {
    exitPosition = pos;
    exitShape.setPosition(pos);
}

void MidChamber::completeChamber() {
    std::cout << "GOT HERE" << std::endl;
    if (!isCompleted && !isFailed) {
        player.setInMidChamber(false);
        player.gainMomentum(15.0f, player.getActiveFormType());
    }
    Chamber::completeChamber();
}

void MidChamber::update(float dt) {
    Chamber::update(dt);
    std::cout << "Updating" << std::endl;

    // MidChamber is a free-switch zone (§6.4): no enemies, no waves.
    // Completion is triggered when the player walks into the exit zone.
    // If no exit position has been set by the factory, fall back to
    // instant completion so the run can still progress.
    if (exitPosition.x < 0.f) {
        // No exit configured — complete immediately (prototype fallback)
        completeChamber();
        return;
    }

    float cellSize = SettingManager::getInstance().getCellSize();
    float dist = Math::distance(player.getPosition(), exitPosition);
    if (dist <= cellSize * 0.6f) {
        std::cout << "MidChamber: Player reached exit.\n";
        completeChamber();
    }
}

void MidChamber::drawForeground(sf::RenderWindow& window) {
    // Draw exit indicator only when an exit has been positioned
    if (exitPosition.x >= 0.f) {
        window.draw(exitShape);
    }
}
