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
    : SpecialAbilityState(inner, 10.0f) {}

StatModifier VoidcasterDetonationFieldState::getStatModifier() const {
    StatModifier modifier;
    
    // TODO (Future): When the combat engine supports 'onHit' events for projectiles,
    // update this state to trigger a 2.5 unit (150 pixel) explosion dealing 0.75x damage
    
    // Placeholder buff: Move 20% faster while the field is active
    modifier.speedMultiplier = 1.2f; 
    
    return modifier;
}

const std::string& VoidcasterDetonationFieldState::getVisualKey() {
    static const std::string key = "VoidcasterDetonationField";
    return key;
}
