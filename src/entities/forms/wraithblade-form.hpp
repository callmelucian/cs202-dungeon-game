#ifndef WRAITHBLADE_FORM_HPP
#define WRAITHBLADE_FORM_HPP

#include "../player-form.hpp"

class WraithbladeForm : public PlayerForm {
public:
    WraithbladeForm();

    void attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) override;
    std::unique_ptr<SpecialAbilityState> createSpecialState(int abilityIndex) override;
};

class WraithbladeRiftcrushState : public SpecialAbilityState {
public:
    WraithbladeRiftcrushState(PlayerCombatState* inner);
    StatModifier getStatModifier() const override;
    const std::string& getVisualKey() override;
    float modifyOutgoingDamage(float baseAmount) override;
};

class WraithbladeCinderveilState : public SpecialAbilityState {
public:
    WraithbladeCinderveilState(PlayerCombatState* inner);
    StatModifier getStatModifier() const override;
    const std::string& getVisualKey() override;
};

#endif // WRAITHBLADE_FORM_HPP
