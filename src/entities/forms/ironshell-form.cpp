#include "ironshell-form.hpp"
#include "../player.hpp"
#include "../effects/paralyzed-effect.hpp"
#include "../../chambers/chamber.hpp"
#include "../../global-settings/sound-manager.hpp"
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
        return std::make_unique<IronshellAegisPulseState>(this);
    } else if (abilityIndex == 2) {
        return std::make_unique<IronshellVeilOfThornsState>(this);
    }
    return nullptr;
}

float IronshellForm::getMomentumGainOnHit(float hpLost) const {
    return hpLost * 1.2f;
}

std::string IronshellForm::getAttackAnimKey() const {
    return "slash-facing-";
}



// ---- IronshellAegisPulseState ----
IronshellAegisPulseState::IronshellAegisPulseState(PlayerCombatState* inner)
    : SpecialAbilityState(inner, 1.0f) {}

void IronshellAegisPulseState::onEnter(Player& player) {
    SpecialAbilityState::onEnter(player);
    Chamber* chamber = player.getChamber();
    if (chamber) {
        onAttack(player, sf::Vector2f(0.0f, 0.0f), *chamber);
    }
}

StatModifier IronshellAegisPulseState::getStatModifier() const {
    return StatModifier{};
}

const std::string& IronshellAegisPulseState::getVisualKey() {
    static const std::string key = "IronshellAegisPulse";
    return key;
}

void IronshellAegisPulseState::onAttack(Player& player, sf::Vector2f targetDir, Chamber& chamber) {
    CircleHitbox shockwave;
    shockwave.center = player.getPosition();
    shockwave.radius = 5.0f * 60.0f; // 5.0 units = 300 pixels
    
    for (auto* enemy : chamber.getEnemiesRaw()) {
        if (enemy && enemy->isAlive()) {
            if (CollisionSolver::checkCollision(shockwave, enemy->getBounds())) {
                float damage = player.getEffectiveStats().damage;
                enemy->takeDamage(damage);
                enemy->applyStatusEffect(std::make_unique<ParalyzedEffect>(1.5f));
                chamber.getItemManager().spawnFragments(enemy->getPosition(), 1);
                chamber.onEnemyHit(enemy, !enemy->isAlive());
            }
        }
    }

    // Emit massive golden shockwave particles
    ParticleSystem::getInstance().emitBurst(player.getPosition(), 45, sf::Color(255, 215, 0, 220), 80.0f, 250.0f, 0.2f, 0.6f, 6.0f);
}

void IronshellAegisPulseState::draw(const Player& player, sf::RenderWindow& window) const {
    SpecialAbilityState::draw(player, window);

    float progress = std::clamp(elapsedTime / duration, 0.0f, 1.0f);
    float maxRadius = 5.0f * 60.0f; // 300px
    float currentRadius = maxRadius * (0.1f + 0.9f * progress);

    // Smooth expanding golden radial pulse
    sf::Color pulseColor(255, 215, 0, 220);
    AuraRenderer::getInstance().drawPulse(window, player.getPosition(), currentRadius, maxRadius, pulseColor, progress);
}

// ---- IronshellVeilOfThornsState ----
IronshellVeilOfThornsState::IronshellVeilOfThornsState(PlayerCombatState* inner) : SpecialAbilityState(inner, 10.0f) {}

StatModifier IronshellVeilOfThornsState::getStatModifier() const {
    return StatModifier{};
}

const std::string& IronshellVeilOfThornsState::getVisualKey() {
    static const std::string key = "IronshellVeilOfThorns";
    return key;
}

void IronshellVeilOfThornsState::update(Player& player, float dt) {
    SpecialAbilityState::update(player, dt);
    
    Chamber* chamber = player.getChamber();
    if (!chamber) return;

    CircleHitbox aura;
    aura.center = player.getPosition();
    aura.radius = 4.0f * 60.0f; // 4.0 units = 240 pixels

    for (auto* enemy : chamber->getEnemiesRaw()) {
        if (!enemy || !enemy->isAlive()) continue;
        if (CollisionSolver::checkCollision(aura, enemy->getBounds())) {
            if (affectedEnemies.find(enemy) == affectedEnemies.end()) {
                affectedEnemies.insert(enemy);
                enemy->applyStatusEffect(std::make_unique<ParalyzedEffect>(1.5f));
                chamber->getItemManager().spawnFragments(enemy->getPosition(), 1);
                ParticleSystem::getInstance().emitBurst(enemy->getPosition(), 15, sf::Color(255, 200, 50, 200), 40.0f, 100.0f, 0.2f, 0.4f, 4.0f);
            }
        }
    }

    // Emit subtle golden ambient sparkles around the aura
    ParticleSystem::getInstance().emitSparkle(player.getPosition(), 2, sf::Color(255, 220, 80, 220), 240.0f);
}

void IronshellVeilOfThornsState::draw(const Player& player, sf::RenderWindow& window) const {
    SpecialAbilityState::draw(player, window);

    float radius = 4.0f * 60.0f; // 240px
    // Smooth radial gradient amber/gold resonance air aura radiating outward from the player
    sf::Color coreColor(255, 235, 100, 110); // Bright golden resonant core
    sf::Color edgeColor(230, 160, 20, 170);  // Deep amber protective boundary air
    AuraRenderer::getInstance().drawAura(window, player.getPosition(), radius, coreColor, edgeColor, 1.0f, 1.1f);
}
