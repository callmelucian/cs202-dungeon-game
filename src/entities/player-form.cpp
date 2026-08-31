#include "player-form.hpp"
#include "player.hpp"
#include "../ui/graphics/aura-renderer.hpp"
#include <algorithm>

// ---- PlayerForm implementation ----
PlayerForm::PlayerForm(FormType formType, const std::string& formName, const Stats& baseStats, float attackRange, float attackRate)
    : formType(formType), formName(formName), baseStats(baseStats), attackRange(attackRange), attackRate(attackRate) {}

void PlayerForm::onEnter(Player& player) {}
void PlayerForm::onExit(Player& player) {}
void PlayerForm::update(Player& player, float dt) {}

void PlayerForm::onAttack(Player& player, sf::Vector2f targetDir, Chamber& chamber) {
    attack(player, targetDir, chamber);
}

float PlayerForm::modifyOutgoingDamage(float baseAmount) {
    return baseAmount;
}

float PlayerForm::modifyIncomingDamage(Player& player, float amount) {
    return amount;
}

float PlayerForm::modifyCriticalRate(float baseRate) {
    return baseRate;
}

void PlayerForm::onEnemyHit(Player& player, Enemy* enemy, bool lethal, Chamber& chamber) {}

Stats PlayerForm::getStats() {
    return baseStats;
}

const std::string& PlayerForm::getVisualKey() {
    return formName;
}

float PlayerForm::getRemainingDuration() {
    return 0.0f;
}

FormType PlayerForm::getFormType() const {
    return formType;
}

const std::string& PlayerForm::getFormName() const {
    return formName;
}

float PlayerForm::getAttackRange() const {
    return attackRange;
}

float PlayerForm::getAttackRate() const {
    return attackRate;
}


// ---- SpecialAbilityState implementation ----
SpecialAbilityState::SpecialAbilityState(PlayerCombatState* inner, float duration, int abilityTier)
    : innerState(inner), duration(duration), elapsedTime(0.0f), abilityTier(abilityTier) {}

void SpecialAbilityState::onEnter(Player& player) {
    if (innerState) innerState->onEnter(player);
}

void SpecialAbilityState::onExit(Player& player) {
    if (innerState) innerState->onExit(player);
}

void SpecialAbilityState::update(Player& player, float dt) {
    elapsedTime += dt;
    if (innerState) innerState->update(player, dt);
}

void SpecialAbilityState::onAttack(Player& player, sf::Vector2f targetDir, Chamber& chamber) {
    if (innerState) innerState->onAttack(player, targetDir, chamber);
}

float SpecialAbilityState::modifyOutgoingDamage(float baseAmount) {
    return innerState ? innerState->modifyOutgoingDamage(baseAmount) : baseAmount;
}

float SpecialAbilityState::modifyIncomingDamage(Player& player, float amount) {
    return innerState ? innerState->modifyIncomingDamage(player, amount) : amount;
}

float SpecialAbilityState::modifyCriticalRate(float baseRate) {
    return innerState ? innerState->modifyCriticalRate(baseRate) : baseRate;
}

void SpecialAbilityState::onEnemyHit(Player& player, Enemy* enemy, bool lethal, Chamber& chamber) {
    if (innerState) innerState->onEnemyHit(player, enemy, lethal, chamber);
}

Stats SpecialAbilityState::getStats() {
    Stats innerStats = innerState ? innerState->getStats() : Stats{};
    return getStatModifier().applyTo(innerStats);
}

float SpecialAbilityState::getRemainingDuration() {
    return std::max(0.0f, duration - elapsedTime);
}

void SpecialAbilityState::draw(const Player& player, sf::RenderWindow& window) const {
    if (innerState) {
        innerState->draw(player, window);
    }

    float auraRadius = 60.0f;
    if (abilityTier == 1) {
        // Gray aura veil for Special 1
        sf::Color grayCore(180, 180, 180, 120);
        sf::Color grayEdge(100, 100, 100, 180);
        AuraRenderer::getInstance().drawAura(window, player.getPosition(), auraRadius, grayCore, grayEdge, 1.1f, 1.3f);
    } else if (abilityTier == 2) {
        // Black aura veil for Special 2
        sf::Color blackCore(50, 50, 50, 180);
        sf::Color blackEdge(10, 10, 10, 240);
        AuraRenderer::getInstance().drawAura(window, player.getPosition(), auraRadius, blackCore, blackEdge, 1.2f, 1.5f);
    }
}


// ---- PlayerCombatStateMachine implementation ----
PlayerCombatStateMachine::PlayerCombatStateMachine()
    : baseState(nullptr), activeState(nullptr) {}

void PlayerCombatStateMachine::setBaseState(PlayerForm* form, Player& player) {
    if (activeState == baseState) {
        if (activeState) {
            activeState->onExit(player);
        }
        baseState = form;
        activeState = baseState;
        if (activeState) {
            activeState->onEnter(player);
        }
    } else {
        baseState = form;
    }
}

void PlayerCombatStateMachine::enterTemporaryState(std::unique_ptr<PlayerCombatState> state, Player& player) {
    if (activeState) {
        activeState->onExit(player);
    }
    ownedTemporaryState = std::move(state);
    activeState = ownedTemporaryState.get();
    if (activeState) {
        activeState->onEnter(player);
    }
}

void PlayerCombatStateMachine::update(Player& player, float dt) {
    if (activeState) {
        activeState->update(player, dt);
        if (ownedTemporaryState && ownedTemporaryState->getRemainingDuration() <= 0.0f) {
            revertToBaseState(player);
        }
    }
}

void PlayerCombatStateMachine::revertToBaseState(Player& player) {
    if (activeState) {
        activeState->onExit(player);
    }
    ownedTemporaryState.reset();
    activeState = baseState;
    if (activeState) {
        activeState->onEnter(player);
    }
}

PlayerCombatState* PlayerCombatStateMachine::getActiveState() const {
    return activeState;
}

bool PlayerCombatStateMachine::isInTemporaryState() const {
    return ownedTemporaryState != nullptr;
}
