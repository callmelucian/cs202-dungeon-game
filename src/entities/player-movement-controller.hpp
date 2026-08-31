// ============================================================
// player-movement-controller.hpp  [GUTTED — movement system overhaul]
//
// Full implementation backed up to: backup-files/player-movement-controller.hpp
// ============================================================
//
// This class previously owned all grid-based movement logic for the player:
//
//   Fields:
//     sf::Vector2f lastDir      — cardinal direction of the last key press;
//                                 used by auto-glide after key release.
//     bool isFacingRight        — updated each frame from input direction.
//     bool wasKeyHeld           — edge-detection flag for key-release event.
//
//   Methods:
//     sf::Vector2f update(Player&, float deltaTime)
//       1. Polls MoveUp/Down/Left/Right key bindings via SettingManager.
//       2. Normalises diagonal input to a unit vector.
//       3. Stores dominant-axis lastDir (never diagonal) to avoid grid drift.
//       4. While moving: calls Math::applyGridAxisSnap() to nudge the player
//          toward the nearest cell-centre line on the perpendicular axis.
//       5. On key release: if distToCenter < 1.5px → snap & stop;
//          else auto-glide in lastDir until reaching the target cell centre.
//       6. Writes final velocity: dir * speed * (cellSize * speedMultiplier).
//       Returns the resolved direction (zero when idle/snapped).
//
//     void onWallCollision()
//       Clears lastDir so auto-glide does not push the player into the wall.
//
//     bool getIsFacingRight() const
//       Returns the current facing flag (used by animation and attack systems).

#ifndef PLAYER_MOVEMENT_CONTROLLER_HPP
#define PLAYER_MOVEMENT_CONTROLLER_HPP

#include <SFML/System/Vector2.hpp>
#include <string>

class Player;

enum class FacingDirection {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

class PlayerMovementController {
public:
    PlayerMovementController() = default;

    sf::Vector2f update(Player& player, float deltaTime);
    void onWallCollision();
    
    FacingDirection getFacingDirection() const;
    sf::Vector2f getFacingVector() const;
    std::string getFacingString() const;
    void setFacingFromVector(const sf::Vector2f& dir);
    bool getIsFacingRight() const;
    sf::Vector2f getLastMoveDir() const { return lastMoveDir; }
    bool isMoving() const;

private:
    FacingDirection facingDirection = FacingDirection::DOWN;
    sf::Vector2f lastMoveDir{0.f, 0.f};
};

#endif // PLAYER_MOVEMENT_CONTROLLER_HPP
