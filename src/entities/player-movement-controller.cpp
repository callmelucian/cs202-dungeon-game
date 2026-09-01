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
        lastMoveDir = sf::Vector2f(0.f, 0.f);
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
    lastMoveDir = moveDir;
    setFacingFromVector(moveDir);
    
    float speedMultiplier = settings.getSpeedMultiplier();
    if (player.isDashing()) {
        speedMultiplier *= 2.0f; // 2x speed during dash
    }
    if (player.isOnStairs()) {
        speedMultiplier *= 0.65f; // Slower movement on stairs for realistic climbing feel
    }
    float speedToPixels = settings.getCellSize() * speedMultiplier;
    player.setVelocity(moveDir * player.getSpeed() * speedToPixels);
    
    return moveDir;
}

bool PlayerMovementController::isMoving() const {
    return (std::abs(lastMoveDir.x) > 0.001f || std::abs(lastMoveDir.y) > 0.001f);
}

void PlayerMovementController::onWallCollision() {
    // Stateless movement: no persistent moving state needs to be cleared.
}

FacingDirection PlayerMovementController::getFacingDirection() const {
    return facingDirection;
}

sf::Vector2f PlayerMovementController::getFacingVector() const {
    switch (facingDirection) {
        case FacingDirection::UP:    return sf::Vector2f(0.0f, -1.0f);
        case FacingDirection::DOWN:  return sf::Vector2f(0.0f, 1.0f);
        case FacingDirection::LEFT:  return sf::Vector2f(-1.0f, 0.0f);
        case FacingDirection::RIGHT: return sf::Vector2f(1.0f, 0.0f);
    }
    return sf::Vector2f(0.0f, 1.0f);
}

std::string PlayerMovementController::getFacingString() const {
    switch (facingDirection) {
        case FacingDirection::UP:    return "up";
        case FacingDirection::DOWN:  return "down";
        case FacingDirection::LEFT:  return "left";
        case FacingDirection::RIGHT: return "right";
    }
    return "down";
}

void PlayerMovementController::setFacingFromVector(const sf::Vector2f& dir) {
    if (std::abs(dir.x) < 0.01f && std::abs(dir.y) < 0.01f) {
        return;
    }
    if (std::abs(dir.x) >= std::abs(dir.y)) {
        facingDirection = (dir.x > 0.f) ? FacingDirection::RIGHT : FacingDirection::LEFT;
    } else {
        facingDirection = (dir.y > 0.f) ? FacingDirection::DOWN : FacingDirection::UP;
    }
}

bool PlayerMovementController::getIsFacingRight() const {
    return facingDirection != FacingDirection::LEFT;
}
