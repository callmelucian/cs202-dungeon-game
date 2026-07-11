#include "voidcaster-form.hpp"
#include "../player.hpp"
#include "../../chambers/chamber.hpp"
#include <cmath>

VoidcasterForm::VoidcasterForm()
    : PlayerForm(FormType::VOIDCASTER, "Voidcaster",
                 Stats{100.0f, 100.0f, 22.0f, 5.0f, 5.0f},
                 12.0f, 1.0f) {}

void VoidcasterForm::attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) {
    // Gain +8 momentum on hit (+4 bonus per additional enemy pierced)
    player.gainMomentum(8.0f, FormType::VOIDCASTER);
    
    // Voidcaster: Ranged (12 units, piercing) - Line shape
    float rangePixels = getAttackRange() * 60.0f; // 12.0 * 60 = 720
    
    float len = std::sqrt(targetDir.x * targetDir.x + targetDir.y * targetDir.y);
    if (len > 0) targetDir /= len;
    else targetDir = sf::Vector2f(1.0f, 0.0f);

    LineHitbox line;
    line.start = player.getPosition();
    line.end = line.start + targetDir * rangePixels;

    chamber.processPlayerAttack(line);
}

std::unique_ptr<SpecialAbilityState> VoidcasterForm::createSpecialState(int abilityIndex) {
    if (abilityIndex == 1) {
        return std::make_unique<VoidcasterLanceState>(this);
    } else if (abilityIndex == 2) {
        return std::make_unique<VoidcasterDetonationFieldState>(this);
    }
    return nullptr;
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


// ---- VoidcasterDetonationFieldState ----
VoidcasterDetonationFieldState::VoidcasterDetonationFieldState(PlayerCombatState* inner)
    : SpecialAbilityState(inner, 10.0f) {}

StatModifier VoidcasterDetonationFieldState::getStatModifier() const {
    return StatModifier{};
}

const std::string& VoidcasterDetonationFieldState::getVisualKey() {
    static const std::string key = "VoidcasterDetonationField";
    return key;
}
