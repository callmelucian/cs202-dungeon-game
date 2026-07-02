#include "ironshell-form.hpp"
#include "../player.hpp"

IronshellForm::IronshellForm()
    : PlayerForm(FormType::IRONSHELL, 2.5f, 160.0f, 6.0f, 1.0f, 1.0f) {}

void IronshellForm::attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) {
    // Stub attack logic
    // Gain +3 momentum on hit
    player.gainMomentum(3.0f, FormType::IRONSHELL);
}

void IronshellForm::triggerSpecial1(Player& player, Chamber& chamber) {
    // Aegis Pulse - Outward shockwave that staggers nearby enemies and knocks out fragments
}

void IronshellForm::triggerSpecial2(Player& player, Chamber& chamber) {
    // Veil of Thorns - Aura applies Paralyzed and knocks out fragments
}
