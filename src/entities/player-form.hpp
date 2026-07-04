#ifndef PLAYER_FORM_HPP
#define PLAYER_FORM_HPP

#include <SFML/System/Vector2.hpp>
#include <string>
#include <memory>
#include "../core/enums.hpp"
#include "stats.hpp"

class Player;
class Chamber;
class SpecialAbilityState;

// State pattern interface
class PlayerCombatState {
public:
    virtual ~PlayerCombatState() = default;
    
    virtual void onEnter(Player& player) = 0;
    virtual void onExit(Player& player) = 0;
    virtual void update(Player& player, float dt) = 0;
    virtual void onAttack(Player& player, sf::Vector2f targetDir, Chamber& chamber) = 0;
    virtual float modifyOutgoingDamage(float baseAmount) = 0;
    virtual float modifyIncomingDamage(Player& player, float amount) = 0;
    virtual Stats getStats() = 0;
    virtual const std::string& getVisualKey() = 0;
    virtual float getRemainingDuration() = 0;
};

// Base strategy/state class
class PlayerForm : public PlayerCombatState {
public:
    PlayerForm(FormType formType, const std::string& formName, const Stats& baseStats, float attackRange, float attackRate);
    virtual ~PlayerForm() = default;

    void onEnter(Player& player) override;
    void onExit(Player& player) override;
    void update(Player& player, float dt) override;
    void onAttack(Player& player, sf::Vector2f targetDir, Chamber& chamber) override;
    float modifyOutgoingDamage(float baseAmount) override;
    float modifyIncomingDamage(Player& player, float amount) override;
    Stats getStats() override;
    const std::string& getVisualKey() override;
    float getRemainingDuration() override;

    virtual void attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) = 0;
    virtual std::unique_ptr<SpecialAbilityState> createSpecialState(int abilityIndex) = 0;

    FormType getFormType() const;
    const std::string& getFormName() const;
    float getAttackRange() const;
    float getAttackRate() const;

protected:
    FormType formType;
    std::string formName;
    Stats baseStats;
    float attackRange;
    float attackRate;
};

// Decorator pattern base class
class SpecialAbilityState : public PlayerCombatState {
public:
    SpecialAbilityState(PlayerCombatState* inner, float duration);
    virtual ~SpecialAbilityState() = default;

    void onEnter(Player& player) override;
    void onExit(Player& player) override;
    void update(Player& player, float dt) override;
    void onAttack(Player& player, sf::Vector2f targetDir, Chamber& chamber) override;
    float modifyOutgoingDamage(float baseAmount) override;
    float modifyIncomingDamage(Player& player, float amount) override;
    Stats getStats() override;
    float getRemainingDuration() override;

    virtual StatModifier getStatModifier() const = 0;
    virtual const std::string& getVisualKey() override = 0;

protected:
    PlayerCombatState* innerState;
    float duration;
    float elapsedTime;
};

// Context machine
class PlayerCombatStateMachine {
public:
    PlayerCombatStateMachine();
    ~PlayerCombatStateMachine() = default;

    void setBaseState(PlayerForm* form, Player& player);
    void enterTemporaryState(std::unique_ptr<PlayerCombatState> state, Player& player);
    void update(Player& player, float dt);
    void revertToBaseState(Player& player);
    
    PlayerCombatState* getActiveState() const;
    bool isInTemporaryState() const;

private:
    PlayerForm* baseState;
    PlayerCombatState* activeState;
    std::unique_ptr<PlayerCombatState> ownedTemporaryState;
};

#endif // PLAYER_FORM_HPP
