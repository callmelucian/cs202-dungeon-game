// ============================================================
// player-movement-controller.cpp
// ============================================================

#include "player-movement-controller.hpp"
#include "player.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../utils/math-utility.hpp"
#include <SFML/Window/Keyboard.hpp>

sf::Vector2f PlayerMovementController::update(Player& player, float deltaTime) {
    if (!player.canAct()) {
        player.setVelocity(sf::Vector2f(0.f, 0.f));
        return sf::Vector2f(0.f, 0.f);
    }

    const auto& settings = SettingManager::getInstance();
    
    sf::Vector2f inputDir(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(settings.getKeyBinding("MoveUp"))) {
        inputDir.y -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(settings.getKeyBinding("MoveDown"))) {
        inputDir.y += 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(settings.getKeyBinding("MoveLeft"))) {
        inputDir.x -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(settings.getKeyBinding("MoveRight"))) {
        inputDir.x += 1.0f;
    }
    
    sf::Vector2f moveDir = Math::normalize(inputDir);
    
    if (moveDir.x < 0.f) {
        isFacingRight = false;
    } else if (moveDir.x > 0.f) {
        isFacingRight = true;
    }
    
    float speedToPixels = settings.getCellSize() * settings.getSpeedMultiplier();
    player.setVelocity(moveDir * player.getSpeed() * speedToPixels);
    
    return moveDir;
}

void PlayerMovementController::onWallCollision() {
    // Stateless movement: no persistent moving state needs to be cleared.
}

bool PlayerMovementController::getIsFacingRight() const {
    return isFacingRight;
}
