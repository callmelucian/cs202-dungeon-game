#include "wraithblade-form.hpp"
#include "../player.hpp"

WraithbladeForm::WraithbladeForm()
    : PlayerForm(FormType::WRAITHBLADE, 7.0f, 100.0f, 12.0f, 1.5f, 2.0f) {}

void WraithbladeForm::attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) {
    // Stub attack logic
    // Gain +6 momentum on hit (assume it hits for test simulation)
    player.gainMomentum(6.0f, FormType::WRAITHBLADE);
}

void WraithbladeForm::triggerSpecial1(Player& player, Chamber& chamber) {
    // Riftcrush - Next knockback strike deals 3x base damage and chains targets
}

void WraithbladeForm::triggerSpecial2(Player& player, Chamber& chamber) {
    // Cinderveil - All hits apply Burned for 10 seconds
}
