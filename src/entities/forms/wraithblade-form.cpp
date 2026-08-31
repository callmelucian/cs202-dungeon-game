#include "wraithblade-form.hpp"
#include "../player.hpp"
#include "../effects/burned-effect.hpp"
#include "../../chambers/chamber.hpp"
#include "../../utils/math-utility.hpp"
#include "../../utils/camera.hpp"
#include "../../global-settings/sound-manager.hpp"
#include "../../ui/graphics/particle-system.hpp"
#include "../../ui/graphics/aura-renderer.hpp"
#include <cmath>

WraithbladeForm::WraithbladeForm()
    : PlayerForm(FormType::WRAITHBLADE, "Wraithblade",
                 Stats{100.0f, 100.0f, 12.0f, 7.0f, 15.0f},
                 1.5f, 2.0f) {}

void WraithbladeForm::update(Player& player, float dt) {
    PlayerForm::update(player, dt);
    timeSinceLastAttack += dt;
}

void WraithbladeForm::attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) {
    SoundManager::getInstance().playSound("swing");

    // Wraithblade: Melee (1.5 units) - Cone shape in target direction
    float rangePixels = getAttackRange() * 60.0f; // 1.5 * 60 = 90
    
    // Normalize targetDir
    float len = std::sqrt(targetDir.x * targetDir.x + targetDir.y * targetDir.y);
    if (len > 0.001f) targetDir /= len;
    else targetDir = player.getFacingVector();

    ConeHitbox cone;
    cone.origin = player.getPosition();
    cone.direction = targetDir;
    cone.length = rangePixels;
    cone.angleDegrees = 90.0f; // 90 degree arc

    int hits = chamber.processPlayerAttack(cone);
    if (hits > 0) {
        // Gain +6 momentum per hit
        player.gainMomentum(6.0f * hits, FormType::WRAITHBLADE);
    }
}

std::unique_ptr<SpecialAbilityState> WraithbladeForm::createSpecialState(int abilityIndex) {
    if (abilityIndex == 1) {
        return std::make_unique<WraithbladeRiftcrushState>(this);
    } else if (abilityIndex == 2) {
        return std::make_unique<WraithbladeCinderveilState>(this);
    }
    return nullptr;
}

float WraithbladeForm::getMomentumGainOnHit(float hpLost) const {
    return hpLost * 0.4f;
}

std::string WraithbladeForm::getAttackAnimKey() const {
    if (timeSinceLastAttack > 1.2f) {
        useReverseSlash = false;
    }
    std::string key = useReverseSlash ? "slash_reverse_oversize-facing-" : "slash_oversize-facing-";
    useReverseSlash = !useReverseSlash;
    timeSinceLastAttack = 0.0f;
    return key;
}



// ---- WraithbladeRiftcrushState ----
WraithbladeRiftcrushState::WraithbladeRiftcrushState(PlayerCombatState* inner)
    : SpecialAbilityState(inner, 999999.0f, 1) {}

StatModifier WraithbladeRiftcrushState::getStatModifier() const {
    return StatModifier{};
}

const std::string& WraithbladeRiftcrushState::getVisualKey() {
    static const std::string key = "WraithbladeRiftcrush";
    return key;
}

float WraithbladeRiftcrushState::modifyOutgoingDamage(float baseAmount) {
    return baseAmount * 2.0f;
}

float WraithbladeRiftcrushState::getRemainingDuration() {
    return (elapsedTime >= duration) ? 0.0f : 1.0f;
}

void WraithbladeRiftcrushState::onAttack(Player& player, sf::Vector2f targetDir, Chamber& chamber) {
    SoundManager::getInstance().playSound("swing");

    // Circular melee with 2x base radius: 2 * (1.5 units * 60px) = 180px radius circle centered on player
    float blastRadius = 3.0f * 60.0f; // 180.0f pixels
    sf::Vector2f playerPos = player.getPosition();

    CircleHitbox circleBlast;
    circleBlast.center = playerPos;
    circleBlast.radius = blastRadius;

    // Strong screen flash and camera shake
    AuraRenderer::getInstance().triggerScreenFlash(sf::Color(255, 255, 255, 240), 0.25f);
    Camera::triggerShake(14.0f, 0.35f);

    // Apply radial 360-degree knockback pushing enemies in all directions away from player
    for (Enemy* enemy : chamber.getEnemiesRaw()) {
        if (enemy && enemy->isAlive()) {
            if (CollisionSolver::checkCollision(circleBlast, enemy->getBounds())) {
                sf::Vector2f dir = enemy->getPosition() - playerPos;
                if (Math::length(dir) > 0.001f) {
                    dir = Math::normalize(dir);
                } else {
                    dir = sf::Vector2f(1.0f, 0.0f);
                }
                enemy->applyKnockback(dir, 1800.0f);
            }
        }
    }

    ParticleSystem::getInstance().emitBurst(playerPos, 50, sf::Color(200, 200, 200, 240), 80.0f, 240.0f, 0.3f, 0.8f, 6.0f);
    chamber.processPlayerAttack(circleBlast);

    // Consume ability only after this first strike is performed
    elapsedTime = duration;
}

// ---- WraithbladeCinderveilState ----
WraithbladeCinderveilState::WraithbladeCinderveilState(PlayerCombatState* inner)
    : SpecialAbilityState(inner, 10.0f, 2) {}

StatModifier WraithbladeCinderveilState::getStatModifier() const {
    return StatModifier{};
}

const std::string& WraithbladeCinderveilState::getVisualKey() {
    static const std::string key = "WraithbladeCinderveil";
    return key;
}

float WraithbladeCinderveilState::modifyOutgoingDamage(float baseAmount) {
    return baseAmount * 2.0f;
}

void WraithbladeCinderveilState::onEnemyHit(Player& player, Enemy* enemy, bool lethal, Chamber& chamber) {
    if (enemy) {
        // Cinderveil: All hits deal 2x damage (via modifyOutgoingDamage), apply Burned, and push enemies back
        if (!lethal) {
            float burnDmgPerSec = player.getEffectiveStats().damage * 0.25f;
            enemy->applyStatusEffect(std::make_unique<BurnedEffect>(burnDmgPerSec, 10.0f));
            ParticleSystem::getInstance().emitBurst(enemy->getPosition(), 20, sf::Color(255, 100, 30, 220), 40.0f, 150.0f, 0.2f, 0.5f, 5.0f);
        }

        if (enemy->canBeKnockedBack()) {
            sf::Vector2f dir = enemy->getPosition() - player.getPosition();
            if (Math::length(dir) > 0.001f) {
                dir = Math::normalize(dir);
            } else {
                dir = player.getFacingVector();
            }
            enemy->applyKnockback(dir, 1600.0f);
        }
    }
    SpecialAbilityState::onEnemyHit(player, enemy, lethal, chamber);
}
