#include "wraithblade-form.hpp"
#include "../player.hpp"

WraithbladeForm::WraithbladeForm()
    : PlayerForm(FormType::WRAITHBLADE, "Wraithblade",
                 Stats{100.0f, 100.0f, 12.0f, 7.0f, 15.0f},
                 1.5f, 2.0f) {}

void WraithbladeForm::attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) {
    // Gain +6 momentum on hit
    player.gainMomentum(6.0f, FormType::WRAITHBLADE);
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
