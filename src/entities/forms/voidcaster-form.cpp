#include "voidcaster-form.hpp"
#include "../player.hpp"
#include "../../chambers/chamber.hpp"
#include "../../utils/math-utility.hpp"
#include "../../global-settings/sound-manager.hpp"
#include "../../ui/graphics/aura-renderer.hpp"
#include "../../core/game.hpp"
#include <cmath>

static sf::Vector2f rotateVector(const sf::Vector2f& v, float angleRad) {
    float cosA = std::cos(angleRad);
    float sinA = std::sin(angleRad);
    return sf::Vector2f(v.x * cosA - v.y * sinA, v.x * sinA + v.y * cosA);
}

VoidcasterForm::VoidcasterForm()
    : PlayerForm(FormType::VOIDCASTER, "Voidcaster",
                 Stats{100.0f, 100.0f, 22.0f, 5.0f, 5.0f},
                 12.0f, 1.0f),
      hitMode(ArrowHitMode::SINGLE_TARGET),
      arrowSpeed(1000.0f) {}

void VoidcasterForm::update(Player& player, float dt) {
    PlayerForm::update(player, dt);

    if (hasPendingArrow) {
        pendingTimer -= dt;
        if (pendingTimer <= 0.0f) {
            hasPendingArrow = false;
            firePendingArrow(player);
        }
    }
}

void VoidcasterForm::attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) {
    float len = std::sqrt(targetDir.x * targetDir.x + targetDir.y * targetDir.y);
    if (len > 0.001f) targetDir /= len;
    else targetDir = player.getFacingVector();

    float maxRange = getAttackRange() * 60.0f;
    pendingTargetDir = targetDir;
    pendingTargetWorldPos = player.getPosition() + targetDir * maxRange;
    pendingChamber = &chamber;
    hasPendingArrow = true;
    pendingTimer = 0.8f; // 8 frames * 0.10s = 0.8s (Frame 8 release)
}

void VoidcasterForm::firePendingArrow(Player& player) {
    if (!pendingChamber) return;
    Chamber& chamber = *pendingChamber;

    // Calculate direction from player position at shooting moment towards target world location
    sf::Vector2f currentPos = player.getPosition();
    sf::Vector2f releaseDir = pendingTargetWorldPos - currentPos;
    float len = std::sqrt(releaseDir.x * releaseDir.x + releaseDir.y * releaseDir.y);
    if (len > 0.001f) {
        releaseDir /= len;
    } else {
        releaseDir = pendingTargetDir;
    }

    // Update facing direction right as arrow releases
    player.setFacingFromVector(releaseDir);

    SoundManager::getInstance().playSound("shoot");

    float maxRange = getAttackRange() * 60.0f; // 12.0 * 60 = 720

    PlayerCombatState* activeState = player.getStateMachine().getActiveState();
    int abilityTier = 0;
    if (player.getStateMachine().isInTemporaryState()) {
        auto* spec = dynamic_cast<SpecialAbilityState*>(activeState);
        if (spec) {
            abilityTier = spec->getAbilityTier();
        }
    }

    if (abilityTier == 1) {
        // Special 1: Triple Volley (3 single-target standard arrows in a narrow spread)
        float anglesDeg[3] = {-6.0f, 0.0f, 6.0f};
        for (float deg : anglesDeg) {
            float rad = deg * 3.1415926535f / 180.0f;
            sf::Vector2f spreadDir = rotateVector(releaseDir, rad);
            chamber.spawnArrow(currentPos, spreadDir, maxRange, arrowSpeed, ArrowHitMode::SINGLE_TARGET, false);
        }
    } else if (abilityTier == 2) {
        // Special 2: Cosmic Laser Burst (3 red laser piercing arrows, 100% crit rate)
        float anglesDeg[3] = {-6.0f, 0.0f, 6.0f};
        for (float deg : anglesDeg) {
            float rad = deg * 3.1415926535f / 180.0f;
            sf::Vector2f spreadDir = rotateVector(releaseDir, rad);
            chamber.spawnArrow(currentPos, spreadDir, maxRange, arrowSpeed, ArrowHitMode::PIERCING, true);
        }
    } else {
        // Standard base attack: 1 single-target arrow
        chamber.spawnArrow(currentPos, releaseDir, maxRange, arrowSpeed, ArrowHitMode::SINGLE_TARGET, false);
    }
}

std::unique_ptr<SpecialAbilityState> VoidcasterForm::createSpecialState(int abilityIndex) {
    if (abilityIndex == 1) {
        return std::make_unique<VoidcasterTripleShotState>(this);
    } else if (abilityIndex == 2) {
        return std::make_unique<VoidcasterCosmicLaserState>(this);
    }
    return nullptr;
}

float VoidcasterForm::getMomentumGainOnHit(float hpLost) const {
    return hpLost * 0.4f;
}

std::string VoidcasterForm::getAttackAnimKey() const {
    return "shoot-facing-";
}


// ---- VoidcasterTripleShotState ----
VoidcasterTripleShotState::VoidcasterTripleShotState(PlayerCombatState* inner)
    : SpecialAbilityState(inner, 10.0f, 1) {}

StatModifier VoidcasterTripleShotState::getStatModifier() const {
    return StatModifier{};
}

const std::string& VoidcasterTripleShotState::getVisualKey() {
    static const std::string key = "VoidcasterTripleShot";
    return key;
}


// ---- VoidcasterCosmicLaserState ----
VoidcasterCosmicLaserState::VoidcasterCosmicLaserState(PlayerCombatState* inner)
    : SpecialAbilityState(inner, 10.0f, 2) {}

StatModifier VoidcasterCosmicLaserState::getStatModifier() const {
    return StatModifier{};
}

const std::string& VoidcasterCosmicLaserState::getVisualKey() {
    static const std::string key = "VoidcasterCosmicLaser";
    return key;
}

float VoidcasterCosmicLaserState::modifyCriticalRate(float baseRate) {
    return 1.0f; // 100% Critical Hit Rate
}
