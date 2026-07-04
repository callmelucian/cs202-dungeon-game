#ifndef VOIDCASTER_FORM_HPP
#define VOIDCASTER_FORM_HPP

#include "../player-form.hpp"

class VoidcasterForm : public PlayerForm {
public:
    VoidcasterForm();

    void attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) override;
    std::unique_ptr<SpecialAbilityState> createSpecialState(int abilityIndex) override;
};

class VoidcasterLanceState : public SpecialAbilityState {
public:
    VoidcasterLanceState(PlayerCombatState* inner);
    StatModifier getStatModifier() const override;
    const std::string& getVisualKey() override;
    float modifyOutgoingDamage(float baseAmount) override;
};

class VoidcasterDetonationFieldState : public SpecialAbilityState {
public:
    VoidcasterDetonationFieldState(PlayerCombatState* inner);
    StatModifier getStatModifier() const override;
    const std::string& getVisualKey() override;
};

#endif // VOIDCASTER_FORM_HPP
