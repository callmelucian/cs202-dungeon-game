#ifndef IRONSHELL_FORM_HPP
#define IRONSHELL_FORM_HPP

#include "../player-form.hpp"

class IronshellForm : public PlayerForm {
public:
    IronshellForm();

    void attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) override;
    std::unique_ptr<SpecialAbilityState> createSpecialState(int abilityIndex) override;
    float getMomentumGainOnHit(float hpLost) const override;
    std::string getAttackAnimKey() const override;
};

#include <unordered_set>

class IronshellParalyzingAuraState : public SpecialAbilityState {
public:
    IronshellParalyzingAuraState(PlayerCombatState* inner);
    StatModifier getStatModifier() const override;
    const std::string& getVisualKey() override;
    void update(Player& player, float dt) override;

private:
    std::unordered_set<class Enemy*> affectedEnemies;
};

class IronshellGlacialConflagrationState : public SpecialAbilityState {
public:
    IronshellGlacialConflagrationState(PlayerCombatState* inner);
    void onEnter(Player& player) override;
    StatModifier getStatModifier() const override;
    const std::string& getVisualKey() override;
};

#endif // IRONSHELL_FORM_HPP
