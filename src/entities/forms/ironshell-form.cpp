#include "ironshell-form.hpp"
#include "../player.hpp"
#include "../effects/paralyzed-effect.hpp"
#include "../effects/slowed-effect.hpp"
#include "../effects/burned-effect.hpp"
#include "../../chambers/chamber.hpp"
#include "../../global-settings/sound-manager.hpp"
#include "../../utils/camera.hpp"
#include "../../ui/graphics/particle-system.hpp"
#include "../../ui/graphics/aura-renderer.hpp"
#include <algorithm>

IronshellForm::IronshellForm()
    : PlayerForm(FormType::IRONSHELL, "Ironshell",
                 Stats{100.0f, 100.0f, 6.0f, 2.5f, 35.0f},
                 1.0f, 1.0f) {}

void IronshellForm::attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) {
    SoundManager::getInstance().playSound("swing");
    
    // Ironshell: Melee (1.0 units, cleave) - Circle shape around player
    float rangePixels = getAttackRange() * 60.0f; // 1.0 * 60 = 60
    
    CircleHitbox circle;
    circle.center = player.getPosition();
    circle.radius = rangePixels;

    int hits = chamber.processPlayerAttack(circle);
    if (hits > 0) {
        // Gain +3 momentum per hit
        player.gainMomentum(3.0f * hits, FormType::IRONSHELL);
    }
}

std::unique_ptr<SpecialAbilityState> IronshellForm::createSpecialState(int abilityIndex) {
    if (abilityIndex == 1) {
        return std::make_unique<IronshellParalyzingAuraState>(this);
    } else if (abilityIndex == 2) {
        return std::make_unique<IronshellGlacialConflagrationState>(this);
    }
    return nullptr;
}

float IronshellForm::getMomentumGainOnHit(float hpLost) const {
    return hpLost * 1.2f;
}

std::string IronshellForm::getAttackAnimKey() const {
    return "slash-facing-";
}


// ---- IronshellParalyzingAuraState (Special 1) ----
IronshellParalyzingAuraState::IronshellParalyzingAuraState(PlayerCombatState* inner)
    : SpecialAbilityState(inner, 10.0f, 1) {}

StatModifier IronshellParalyzingAuraState::getStatModifier() const {
    return StatModifier{};
}

const std::string& IronshellParalyzingAuraState::getVisualKey() {
    static const std::string key = "IronshellParalyzingAura";
    return key;
}

void IronshellParalyzingAuraState::update(Player& player, float dt) {
    SpecialAbilityState::update(player, dt);
    
    Chamber* chamber = player.getChamber();
    if (!chamber) return;

    CircleHitbox aura;
    aura.center = player.getPosition();
    aura.radius = 4.0f * 60.0f; // 4.0 units = 240 pixels

    for (auto* enemy : chamber->getEnemiesRaw()) {
        if (!enemy || !enemy->isAlive()) continue;
        if (CollisionSolver::checkCollision(aura, enemy->getBounds())) {
            enemy->applyStatusEffect(std::make_unique<ParalyzedEffect>(1.5f));
            enemy->applyStatusEffect(std::make_unique<SlowedEffect>(1.5f));

            if (affectedEnemies.find(enemy) == affectedEnemies.end()) {
                affectedEnemies.insert(enemy);
                chamber->getItemManager().spawnFragments(enemy->getPosition(), 1);
                ParticleSystem::getInstance().emitBurst(enemy->getPosition(), 15, sf::Color(200, 200, 200, 220), 40.0f, 100.0f, 0.2f, 0.4f, 4.0f);
            }
        }
    }
}


// ---- IronshellGlacialConflagrationState (Special 2) ----
IronshellGlacialConflagrationState::IronshellGlacialConflagrationState(PlayerCombatState* inner)
    : SpecialAbilityState(inner, 1.0f, 2) {}

void IronshellGlacialConflagrationState::onEnter(Player& player) {
    SpecialAbilityState::onEnter(player);
    Chamber* chamber = player.getChamber();
    if (!chamber) return;

    // Immediately freeze all enemies in the chamber for 7 seconds
    chamber->freezeAllEnemies(7.0f);

    // Apply Burned effect to all active enemies
    float burnDmgPerSec = player.getEffectiveStats().damage * 0.5f;
    for (auto* enemy : chamber->getEnemiesRaw()) {
        if (enemy && enemy->isAlive()) {
            enemy->applyStatusEffect(std::make_unique<BurnedEffect>(burnDmgPerSec, 10.0f));
            ParticleSystem::getInstance().emitBurst(enemy->getPosition(), 25, sf::Color(255, 100, 30, 220), 50.0f, 150.0f, 0.2f, 0.5f, 5.0f);
        }
    }

    // Screen flash and camera shake
    AuraRenderer::getInstance().triggerScreenFlash(sf::Color(100, 220, 255, 230), 0.25f);
    Camera::triggerShake(12.0f, 0.35f);

    // Consumed instantly
    elapsedTime = duration;
}

StatModifier IronshellGlacialConflagrationState::getStatModifier() const {
    return StatModifier{};
}

const std::string& IronshellGlacialConflagrationState::getVisualKey() {
    static const std::string key = "IronshellGlacialConflagration";
    return key;
}
