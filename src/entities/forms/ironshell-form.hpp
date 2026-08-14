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

class IronshellAegisPulseState : public SpecialAbilityState {
public:
    IronshellAegisPulseState(PlayerCombatState* inner);
    void onEnter(Player& player) override;
    StatModifier getStatModifier() const override;
    const std::string& getVisualKey() override;
    void onAttack(Player& player, sf::Vector2f targetDir, Chamber& chamber) override;
    void draw(const Player& player, sf::RenderWindow& window) const override;
};

#include <unordered_set>

class IronshellVeilOfThornsState : public SpecialAbilityState {
public:
    IronshellVeilOfThornsState(PlayerCombatState* inner);
    StatModifier getStatModifier() const override;
    const std::string& getVisualKey() override;
    void update(Player& player, float dt) override;
    void draw(const Player& player, sf::RenderWindow& window) const override;

private:
    std::unordered_set<class Enemy*> affectedEnemies;
};

#endif // IRONSHELL_FORM_HPP
