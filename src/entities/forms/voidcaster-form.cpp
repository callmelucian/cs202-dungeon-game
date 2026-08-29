#include "voidcaster-form.hpp"
#include "../player.hpp"
#include "../../chambers/chamber.hpp"
#include "../../utils/math-utility.hpp"
#include "../../global-settings/sound-manager.hpp"
#include "../../ui/graphics/aura-renderer.hpp"
#include "../../core/game.hpp"
#include <cmath>

VoidcasterForm::VoidcasterForm()
    : PlayerForm(FormType::VOIDCASTER, "Voidcaster",
                 Stats{100.0f, 100.0f, 22.0f, 5.0f, 5.0f},
                 12.0f, 1.0f) {}

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
    if (len > 0) targetDir /= len;
    else targetDir = sf::Vector2f(1.0f, 0.0f);

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
    chamber.spawnArrow(currentPos, releaseDir, maxRange, arrowSpeed, hitMode);
}

std::unique_ptr<SpecialAbilityState> VoidcasterForm::createSpecialState(int abilityIndex) {
    if (abilityIndex == 1) {
        return std::make_unique<VoidcasterLanceState>(this);
    } else if (abilityIndex == 2) {
        return std::make_unique<VoidcasterDetonationFieldState>(this);
    }
    return nullptr;
}

float VoidcasterForm::getMomentumGainOnHit(float hpLost) const {
    return hpLost * 0.4f;
}

std::string VoidcasterForm::getAttackAnimKey() const {
    return "shoot-facing-";
}



// ---- VoidcasterLanceState ----
VoidcasterLanceState::VoidcasterLanceState(PlayerCombatState* inner)
    : SpecialAbilityState(inner, 1.0f) {}

StatModifier VoidcasterLanceState::getStatModifier() const {
    return StatModifier{};
}

const std::string& VoidcasterLanceState::getVisualKey() {
    static const std::string key = "VoidcasterLance";
    return key;
}

float VoidcasterLanceState::modifyOutgoingDamage(float baseAmount) {
    return baseAmount * 2.5f;
}

void VoidcasterLanceState::onAttack(Player& player, sf::Vector2f targetDir, Chamber& chamber) {
    float len = std::sqrt(targetDir.x * targetDir.x + targetDir.y * targetDir.y);
    if (len > 0) targetDir /= len;
    else targetDir = sf::Vector2f(1.0f, 0.0f);

    LineHitbox laser;
    laser.start = player.getPosition();
    laser.end = laser.start + (targetDir * 2000.0f); // 2000 pixels long!
    
    // TODO (Future): Update CollisionSolver to ignore wall collisions for this specific attack
    chamber.processPlayerAttack(laser);

    elapsedTime = duration; 
}

// ---- VoidcasterDetonationFieldState ----
VoidcasterDetonationFieldState::VoidcasterDetonationFieldState(PlayerCombatState* inner)
    : SpecialAbilityState(inner, 10.0f), isExplosionActive(false) {}

StatModifier VoidcasterDetonationFieldState::getStatModifier() const {
    return StatModifier{};
}

const std::string& VoidcasterDetonationFieldState::getVisualKey() {
    static const std::string key = "VoidcasterDetonationField";
    return key;
}

#include "../../ui/graphics/particle-system.hpp"

void VoidcasterDetonationFieldState::onEnemyHit(Player& player, Enemy* enemy, bool lethal, Chamber& chamber) {
    if (enemy && !isExplosionActive) {
        isExplosionActive = true;
        
        // Detonation Field: 2.5 unit (150 pixel) radius explosion dealing 0.75x base damage
        CircleHitbox explosion;
        explosion.center = enemy->getPosition();
        explosion.radius = 2.5f * 60.0f; // 150.0f pixels

        float expDamage = player.getEffectiveStats().damage * 0.75f;
        
        for (auto* targetEnemy : chamber.getEnemiesRaw()) {
            if (targetEnemy && targetEnemy->isAlive()) {
                if (CollisionSolver::checkCollision(explosion, targetEnemy->getBounds())) {
                    targetEnemy->takeDamage(expDamage);
                    bool targetLethal = !targetEnemy->isAlive();
                    chamber.onEnemyHit(targetEnemy, targetLethal);
                }
            }
        }
        
        ParticleSystem::getInstance().emitBurst(enemy->getPosition(), 30, sf::Color(180, 80, 255, 220), 50.0f, 180.0f, 0.2f, 0.5f, 5.0f);
        isExplosionActive = false;
    }
    SpecialAbilityState::onEnemyHit(player, enemy, lethal, chamber);
}

void VoidcasterDetonationFieldState::draw(const Player& player, sf::RenderWindow& window) const {
    SpecialAbilityState::draw(player, window);

    float radius = 2.5f * 60.0f; // 150px
    // Smooth radial gradient void cosmic air aura radiating outward from the player
    sf::Color coreColor(190, 90, 255, 120);  // Ethereal glowing magenta-violet core
    sf::Color edgeColor(120, 20, 210, 180);  // Deep cosmic void edge air
    AuraRenderer::getInstance().drawAura(window, player.getPosition(), radius, coreColor, edgeColor, 1.05f, 1.25f);
}
