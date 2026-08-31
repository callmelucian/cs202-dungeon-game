#ifndef WRAITHBLADE_FORM_HPP
#define WRAITHBLADE_FORM_HPP

#include "../player-form.hpp"

class WraithbladeForm : public PlayerForm {
public:
    WraithbladeForm();

    void update(Player& player, float dt) override;
    void attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) override;
    std::unique_ptr<SpecialAbilityState> createSpecialState(int abilityIndex) override;
    float getMomentumGainOnHit(float hpLost) const override;
    std::string getAttackAnimKey() const override;

private:
    mutable bool useReverseSlash = false;
    mutable float timeSinceLastAttack = 0.0f;
};

class WraithbladeRiftcrushState : public SpecialAbilityState {
public:
    WraithbladeRiftcrushState(PlayerCombatState* inner);
    StatModifier getStatModifier() const override;
    const std::string& getVisualKey() override;
    float modifyOutgoingDamage(float baseAmount) override;
    void onAttack(Player& player, sf::Vector2f targetDir, Chamber& chamber) override;
    float getRemainingDuration() override;
};

class WraithbladeCinderveilState : public SpecialAbilityState {
public:
    WraithbladeCinderveilState(PlayerCombatState* inner);
    StatModifier getStatModifier() const override;
    const std::string& getVisualKey() override;
    float modifyOutgoingDamage(float baseAmount) override;
    void onEnemyHit(Player& player, Enemy* enemy, bool lethal, Chamber& chamber) override;
};

#endif // WRAITHBLADE_FORM_HPP