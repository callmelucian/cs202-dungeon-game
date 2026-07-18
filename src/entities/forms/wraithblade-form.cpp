#include "wraithblade-form.hpp"
#include "../player.hpp"
#include "../../chambers/chamber.hpp"
#include "../../utils/math-utility.hpp"
#include <cmath>

WraithbladeForm::WraithbladeForm()
    : PlayerForm(FormType::WRAITHBLADE, "Wraithblade",
                 Stats{100.0f, 100.0f, 12.0f, 7.0f, 15.0f},
                 1.5f, 2.0f) {}

void WraithbladeForm::attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) {
    // Gain +5 momentum on hit
    player.gainMomentum(5.0f, FormType::WRAITHBLADE);

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

    chamber.processPlayerAttack(cone);
}

std::unique_ptr<SpecialAbilityState> WraithbladeForm::createSpecialState(int abilityIndex) {
    if (abilityIndex == 1) {
        return std::make_unique<WraithbladeRiftcrushState>(this);
    } else if (abilityIndex == 2) {
        return std::make_unique<WraithbladeCinderveilState>(this);
    }
    return nullptr;
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
    CircularHitbox bigBlast;
    bigBlast.origin = player.getPosition();
    bigBlast.radius = 180.0f; // 3 units * 60 pixels = 180
    
    chamber.processPlayerAttack(bigBlast);

    // Consume the ability instantly so it only works for the "Next strike"
    elapsedTime = duration; 
}

// ---- WraithbladeCinderveilState ----
WraithbladeCinderveilState::WraithbladeCinderveilState(PlayerCombatState* inner)
    : SpecialAbilityState(inner, 10.0f) {}

StatModifier WraithbladeCinderveilState::getStatModifier() const {
    StatModifier modifier;
    
    // TODO (Future): When the combat engine supports status effects, 
    // update this state so that attacks apply the "Burned" status to enemies!
    
    // Placeholder buff: Attack 50% faster while Cinderveil is active
    modifier.attackRateMultiplier = 1.5f; 
    
    return modifier;
}

const std::string& WraithbladeCinderveilState::getVisualKey() {
    static const std::string key = "WraithbladeCinderveil";
    return key;
}
