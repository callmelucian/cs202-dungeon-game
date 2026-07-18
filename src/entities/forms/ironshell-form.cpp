#include "ironshell-form.hpp"
#include "../player.hpp"
#include "../../chambers/chamber.hpp"

IronshellForm::IronshellForm()
    : PlayerForm(FormType::IRONSHELL, "Ironshell",
                 Stats{100.0f, 100.0f, 6.0f, 2.5f, 35.0f},
                 1.0f, 1.0f) {}

void IronshellForm::attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) {
    // Gain +3 momentum on hit
    player.gainMomentum(3.0f, FormType::IRONSHELL);
    
    // Ironshell: Melee (1.0 units, cleave) - Circle shape around player
    float rangePixels = getAttackRange() * 60.0f; // 1.0 * 60 = 60
    
    CircleHitbox circle;
    circle.center = player.getPosition();
    circle.radius = rangePixels;

    chamber.processPlayerAttack(circle);
}

std::unique_ptr<SpecialAbilityState> IronshellForm::createSpecialState(int abilityIndex) {
    if (abilityIndex == 1) {
        return std::make_unique<IronshellAegisPulseState>(this);
    } else if (abilityIndex == 2) {
        return std::make_unique<IronshellVeilOfThornsState>(this);
    }
    return nullptr;
}


// ---- IronshellAegisPulseState ----
IronshellAegisPulseState::IronshellAegisPulseState(PlayerCombatState* inner)
    : SpecialAbilityState(inner, 1.0f) {}

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
    shockwave.radius = 300.0f;
    
    // TODO (Future): Update this attack to apply "Paralyzed" and knock out 1 Echo Fragment from enemies!
    chamber.processPlayerAttack(shockwave);

    elapsedTime = duration;
}

// ---- IronshellVeilOfThornsState ----
IronshellVeilOfThornsState::IronshellVeilOfThornsState(PlayerCombatState* inner) : SpecialAbilityState(inner, 10.0f) {}

StatModifier IronshellVeilOfThornsState::getStatModifier() const {
    StatModifier modifier;
    
    // TODO (Future): Make the Ironshell Aura apply "Paralyzed" and knock out Echo Fragments!
    
    // Placeholder buff: Gain +50 extra defense while active
    modifier.hpMultiplier = 1.5f;  
    
    return modifier;
}

const std::string& IronshellVeilOfThornsState::getVisualKey() {
    static const std::string key = "IronshellVeilOfThorns";
    return key;
}
