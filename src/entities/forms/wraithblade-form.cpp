#include "wraithblade-form.hpp"
#include "../player.hpp"
#include "../effects/burned-effect.hpp"
#include "../../chambers/chamber.hpp"
#include "../../utils/math-utility.hpp"
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
    if (len > 0) targetDir /= len;
    else targetDir = sf::Vector2f(1.0f, 0.0f);

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
    : SpecialAbilityState(inner, 1.0f) {}

StatModifier WraithbladeRiftcrushState::getStatModifier() const {
    return StatModifier{};
}

const std::string& WraithbladeRiftcrushState::getVisualKey() {
    static const std::string key = "WraithbladeRiftcrush";
    return key;
}

float WraithbladeRiftcrushState::modifyOutgoingDamage(float baseAmount) {
    return baseAmount * 3.0f;
}

void WraithbladeRiftcrushState::onAttack(Player& player, sf::Vector2f targetDir, Chamber& chamber) {
    SoundManager::getInstance().playSound("swing");

    // Primary strike in front of player
    float rangePixels = 90.0f; // 1.5 units * 60 pixels = 90
    float len = std::sqrt(targetDir.x * targetDir.x + targetDir.y * targetDir.y);
    if (len > 0) targetDir /= len;
    else targetDir = sf::Vector2f(1.0f, 0.0f);

    // Target detonation center at enemy in range or forward target point
    sf::Vector2f blastCenter = player.getPosition() + targetDir * (rangePixels * 0.75f);
    for (Enemy* enemy : chamber.getEnemiesRaw()) {
        if (enemy && enemy->isAlive()) {
            if (Math::distance(player.getPosition(), enemy->getPosition()) <= rangePixels + 25.0f) {
                blastCenter = enemy->getPosition();
                break;
            }
        }
    }

    // Detonate 180px Riftcrush AoE shockwave from target location
    CircleHitbox bigBlast;
    bigBlast.center = blastCenter;
    bigBlast.radius = 180.0f; // 3 units * 60 pixels = 180
    
    ParticleSystem::getInstance().emitBurst(blastCenter, 40, sf::Color(180, 50, 255, 220), 50.0f, 200.0f, 0.3f, 0.7f, 6.0f);
    chamber.processPlayerAttack(bigBlast);

    // Consume the ability instantly so it only works for the "Next strike"
    elapsedTime = duration; 
}

// ---- WraithbladeCinderveilState ----
WraithbladeCinderveilState::WraithbladeCinderveilState(PlayerCombatState* inner)
    : SpecialAbilityState(inner, 10.0f) {}

StatModifier WraithbladeCinderveilState::getStatModifier() const {
    return StatModifier{};
}

const std::string& WraithbladeCinderveilState::getVisualKey() {
    static const std::string key = "WraithbladeCinderveil";
    return key;
}

#include "../../ui/graphics/particle-system.hpp"

void WraithbladeCinderveilState::onEnemyHit(Player& player, Enemy* enemy, bool lethal, Chamber& chamber) {
    if (!lethal && enemy) {
        // Cinderveil: All hits apply Burned (0.25 * base damage = 3.0 dmg/sec for 10 seconds)
        float burnDmgPerSec = player.getEffectiveStats().damage * 0.25f;
        enemy->applyStatusEffect(std::make_unique<BurnedEffect>(burnDmgPerSec, 10.0f));
        ParticleSystem::getInstance().emitBurst(enemy->getPosition(), 20, sf::Color(255, 100, 30, 220), 40.0f, 150.0f, 0.2f, 0.5f, 5.0f);
    }
    SpecialAbilityState::onEnemyHit(player, enemy, lethal, chamber);
}

void WraithbladeCinderveilState::draw(const Player& player, sf::RenderWindow& window) const {
    SpecialAbilityState::draw(player, window);

    float radius = 1.5f * 60.0f; // 90px
    // Smooth radial gradient thermal flame air aura radiating outward from the player
    sf::Color coreColor(255, 140, 30, 120); // Warm gold-orange incandescent core
    sf::Color edgeColor(255, 50, 10, 180);  // Fiery crimson outward thermal wave
    AuraRenderer::getInstance().drawAura(window, player.getPosition(), radius, coreColor, edgeColor, 1.1f, 1.4f);

    // Emit subtle flame sparkles
    ParticleSystem::getInstance().emitSparkle(player.getPosition(), 2, sf::Color(255, 150, 40, 220), 90.0f);
}
