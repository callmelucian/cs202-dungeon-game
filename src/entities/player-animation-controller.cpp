// ============================================================
// player-animation-controller.cpp  [GUTTED — movement system overhaul]
//
// Full implementation backed up to: backup-files/player-animation-controller.cpp
// ============================================================

#include "player-animation-controller.hpp"
#include "player.hpp"

void PlayerAnimationController::updateMovementAnim(Player& player,
                                                   const sf::Vector2f& dir,
                                                   bool isFacingRight) {
    if (attacking) return; // don't interrupt attack clip
    std::string facing = isFacingRight ? "right" : "left";
    
    // Check if the vector is practically zero
    if (std::abs(dir.x) > 0.01f || std::abs(dir.y) > 0.01f) {
        player.triggerAnimation("run-facing-" + facing);
    } else {
        player.triggerAnimation("idle-facing-" + facing);
    }
}

void PlayerAnimationController::triggerAttackAnim(Player& player, bool isFacingRight) {
    std::string facing = isFacingRight ? "right" : "left";
    std::string animName = player.getActiveForm() 
                           ? player.getActiveForm()->getAttackAnimKey() 
                           : "slash-facing-";
    attacking = true;
    player.triggerAnimation(animName + facing);
}

bool PlayerAnimationController::tickAttackFinished(Player& player) {
    if (!attacking) return false;
    if (player.isAnimationFinished()) {
        attacking = false;
        return true;
    }
    return false;
}

bool PlayerAnimationController::isAttacking() const {
    return attacking;
}
