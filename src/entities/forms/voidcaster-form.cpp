#include "voidcaster-form.hpp"
#include "../player.hpp"

VoidcasterForm::VoidcasterForm()
    : PlayerForm(FormType::VOIDCASTER, 5.0f, 70.0f, 22.0f, 12.0f, 1.0f) {}

void VoidcasterForm::attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) {
    // Stub attack logic
    // Gain +8 momentum on hit (+4 bonus per additional enemy pierced)
    player.gainMomentum(8.0f, FormType::VOIDCASTER);
}

void VoidcasterForm::triggerSpecial1(Player& player, Chamber& chamber) {
    // Lance of the Hollow - Straight line charged shot for 2.5x base damage
}

void VoidcasterForm::triggerSpecial2(Player& player, Chamber& chamber) {
    // Detonation Field - Landed shots trigger explosions for 0.75x base damage
}
