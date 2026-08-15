// ============================================================
// player-animation-controller.hpp  [GUTTED — movement system overhaul]
//
// Full implementation backed up to: backup-files/player-animation-controller.hpp
// ============================================================
//
// This class previously owned all movement/attack animation selection for the player:
//
//   Fields:
//     bool attacking    — true while an attack clip is playing; suppresses
//                         idle/run animations until the clip finishes.
//
//   Methods:
//     void updateMovementAnim(Player&, const sf::Vector2f& dir, bool isFacingRight)
//       Called every frame after movement direction is resolved.
//       No-ops while attacking == true.
//       Fires via player.triggerAnimation():
//         dir != zero  ->  "run-facing-right"  or "run-facing-left"
//         dir == zero  ->  "idle-facing-right" or "idle-facing-left"
//
//     void triggerAttackAnim(Player&, bool isFacingRight)
//       Sets attacking = true, then fires player.triggerAnimation():
//         key = player.getActiveForm()->getAttackAnimKey() + facing
//       Form key mapping (no FormType switch — delegated to the form):
//         WraithbladeForm  ->  "backslash-facing-{right|left}"
//         VoidcasterForm   ->  "shoot-facing-{right|left}"
//         IronshellForm    ->  "slash-facing-{right|left}"
//
//     bool tickAttackFinished(Player&)
//       Polls player.isAnimationFinished() (-> CharacterAnimator::isCurrentAnimationFinished).
//       Clears attacking when the clip ends. Returns true on the frame it clears.
//       Must be called once per frame BEFORE updateMovementAnim.

#ifndef PLAYER_ANIMATION_CONTROLLER_HPP
#define PLAYER_ANIMATION_CONTROLLER_HPP

#include <SFML/System/Vector2.hpp>
#include <string>

class Player;

class PlayerAnimationController {
public:
    PlayerAnimationController() = default;

    // TODO: Implement new animation system.
    // See backup-files/player-animation-controller.cpp for the previous implementation.
    void updateMovementAnim(Player& player, const sf::Vector2f& dir, const std::string& facing);
    void updateMovementAnim(Player& player, const sf::Vector2f& dir, bool isFacingRight);
    void triggerAttackAnim(Player& player, const std::string& facing);
    void triggerAttackAnim(Player& player, bool isFacingRight);
    bool tickAttackFinished(Player& player);
    bool isAttacking() const;

private:
    bool attacking = false;
};

#endif // PLAYER_ANIMATION_CONTROLLER_HPP
