#include "wraithblade-form.hpp"
#include "../player.hpp"
#include "../effects/burned-effect.hpp"
#include "../../chambers/chamber.hpp"
#include "../../utils/math-utility.hpp"
#include "../../global-settings/sound-manager.hpp"
#include <cmath>

WraithbladeForm::WraithbladeForm()
    : PlayerForm(FormType::WRAITHBLADE, "Wraithblade",
                 Stats{100.0f, 100.0f, 12.0f, 7.0f, 15.0f},
                 1.5f, 2.0f) {}

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
    return "backslash-facing-";
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
    // TODO (Future): Change this to trigger the explosion from the ENEMY'S location instead of the Player
    CircleHitbox bigBlast;
    bigBlast.center = player.getPosition();
    bigBlast.radius = 120.0f; // 3 units * 60 pixels = 180
    
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

#include "../../graphics/particle-system.hpp"

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
    sf::CircleShape flameAura(radius);
    flameAura.setOrigin({radius, radius});
    flameAura.setPosition(player.getPosition());

    // Glowing fiery orange flame aura ring
    flameAura.setFillColor(sf::Color(255, 80, 20, 35));
    flameAura.setOutlineColor(sf::Color(255, 120, 30, 220));
    flameAura.setOutlineThickness(3.0f);

    window.draw(flameAura);

    // Emit subtle flame sparkles
    ParticleSystem::getInstance().emitSparkle(player.getPosition(), 2, sf::Color(255, 150, 40, 220), 90.0f);
}
