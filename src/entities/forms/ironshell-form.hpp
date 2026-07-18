#ifndef IRONSHELL_FORM_HPP
#define IRONSHELL_FORM_HPP

#include "../player-form.hpp"

class IronshellForm : public PlayerForm {
public:
    IronshellForm();

    void attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) override;
    std::unique_ptr<SpecialAbilityState> createSpecialState(int abilityIndex) override;
};

class IronshellAegisPulseState : public SpecialAbilityState {
public:
    IronshellAegisPulseState(PlayerCombatState* inner);
    StatModifier getStatModifier() const override;
    const std::string& getVisualKey() override;
    void onAttack(Player& player, sf::Vector2f targetDir, Chamber& chamber) override;
};

class IronshellVeilOfThornsState : public SpecialAbilityState {
public:
    IronshellVeilOfThornsState(PlayerCombatState* inner);
    StatModifier getStatModifier() const override;
    const std::string& getVisualKey() override;
};

#endif // IRONSHELL_FORM_HPP
