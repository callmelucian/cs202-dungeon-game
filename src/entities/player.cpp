#include "player.hpp"
#include "../utils/math-utility.hpp"
#include "../global-settings/setting-manager.hpp"
#include "animation/character-animator.hpp"
#include <algorithm>
#include <map>
#include <iostream>
#include <string>
#include "../chambers/chamber.hpp"
#include "effects/slowed-effect.hpp"
#include "../global-settings/sound-manager.hpp"

Player::Player(PlayableCharacter& character)
    : Character(character.getName()),
      character(&character),
      activeForm(nullptr),
      switchCooldownTimer(0.0f),
      isSwitchCooldownEnabled(true)
{
    // Create forms using the abstract factory
    forms[FormType::WRAITHBLADE] = character.createForm1();
    forms[FormType::VOIDCASTER] = character.createForm2();
    forms[FormType::IRONSHELL] = character.createForm3();

    // Initialize momentum map
    formMomentum[FormType::WRAITHBLADE] = 0.0f;
    formMomentum[FormType::VOIDCASTER] = 0.0f;
    formMomentum[FormType::IRONSHELL] = 0.0f;

    // Set default form
    auto it = forms.find(FormType::WRAITHBLADE);
    if (it != forms.end()) {
        activeForm = it->second.get();
        stateMachine.setBaseState(activeForm, *this);
    }

    // Set initial character baseStats
    if (activeForm) {
        baseStats = activeForm->getStats();
        baseStats.hp = baseStats.maxHp;
        
        if (animator) {
            animator->setCharacterKey(this->character->getName() + "_" + activeForm->getVisualKey());
        }
    }

    setHealthBar(std::make_unique<UI::PlayerHealthBar>());
}

void Player::handleInput(const sf::Event& event) {
    SettingManager& settings = SettingManager::getInstance();

    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->scancode == settings.getKeyBinding("SwitchForm1")) {
            switchForm(FormType::WRAITHBLADE);
        } else if (keyEvent->scancode == settings.getKeyBinding("SwitchForm2")) {
            switchForm(FormType::VOIDCASTER);
        } else if (keyEvent->scancode == settings.getKeyBinding("SwitchForm3")) {
            switchForm(FormType::IRONSHELL);
        } else if (keyEvent->scancode == settings.getKeyBinding("Special1")) {
            if (currentChamber) triggerSpecial(1, *currentChamber);
        } else if (keyEvent->scancode == settings.getKeyBinding("Special2")) {
            if (currentChamber) triggerSpecial(2, *currentChamber);
        }
    }
}

void Player::triggerAnimation(const std::string& key) {
    notifyStateChanged(key);
}

void Player::update(float deltaTime) {
    animController.tickAttackFinished(*this);
    sf::Vector2f dir = movementController.update(*this, deltaTime);
    animController.updateMovementAnim(*this, dir, movementController.getFacingString());

    // 3. Update pending attack release (for archery bow windup / release)
    if (hasPendingAttack) {
        attackReleaseTimer -= deltaTime;
        if (attackReleaseTimer <= 0.0f) {
            hasPendingAttack = false;
            if (stateMachine.getActiveState() && currentChamber) {
                stateMachine.getActiveState()->onAttack(*this, pendingAttackDir, *currentChamber);
            }
        }
    }

    // 4. Update switch cooldown
    if (switchCooldownTimer > 0.0f) {
        switchCooldownTimer -= deltaTime;
        if (switchCooldownTimer < 0.0f) {
            switchCooldownTimer = 0.0f;
        }
    }

    // 5. State machine update (updates active combat state)
    stateMachine.update(*this, deltaTime);

    // 6. Character base update (ticks status effects, knockback, health bar)
    Character::update(deltaTime);

    // 7. Apply SlowAura if Ironshell
    if (getActiveFormType() == FormType::IRONSHELL && currentChamber) {
        auto enemies = currentChamber->getEnemiesRaw();
        applySlowAura(enemies);
    }

    // 8. Sync momentum onto health bar
    if (healthBar) {
        healthBar->setMomentum(getMomentum(getActiveFormType()), special1Threshold);
    }
}

void Player::draw(sf::RenderWindow& window) const {
    Character::draw(window);

    if (stateMachine.getActiveState()) {
        stateMachine.getActiveState()->draw(*this, window);
    }
}


void Player::setChamber(Chamber* chamber) {
    this->currentChamber = chamber;
}

void Player::applySlowAura(std::vector<Enemy*>& enemies) {
    float cellSize = SettingManager::getInstance().getCellSize();
    float auraRadius = 4.0f * cellSize;
    
    for (auto* enemy : enemies) {
        if (!enemy || !enemy->isAlive()) continue;
        
        float dist = Math::distance(getPosition(), enemy->getPosition());
        if (dist <= auraRadius) {
            enemy->applyStatusEffect(std::make_unique<SlowedEffect>(0.5f));
        }
    }
}

void Player::takeDamage(float rawAmount) {
    // 1. Modify incoming damage via active state (Decorator)
    float modifiedAmount = rawAmount;
    if (stateMachine.getActiveState()) {
        modifiedAmount = stateMachine.getActiveState()->modifyIncomingDamage(*this, rawAmount);
    }

    // 2. Apply mitigated damage to base HP
    float oldHp = baseStats.hp;
    Character::takeDamage(modifiedAmount);
    float actualHpLost = oldHp - baseStats.hp;

    // 3. Gain momentum — rate is defined by the active form (no FormType switch needed)
    if (actualHpLost > 0.0f && activeForm) {
        gainMomentum(activeForm->getMomentumGainOnHit(actualHpLost), getActiveFormType());
    }
}


bool Player::switchForm(FormType newForm) {
    if (activeForm && activeForm->getFormType() == newForm) return false;
    if (isSwitchCooldownEnabled && switchCooldownTimer > 0.0f) return false;

    hasPendingAttack = false;

    // Reset momentum of form we are switching away from
    if (activeForm) formMomentum[activeForm->getFormType()] = 0.0f;

    auto it = forms.find(newForm);
    if (it != forms.end()) {
        activeForm = it->second.get();
        stateMachine.setBaseState(activeForm, *this);

        baseStats.damage = activeForm->getStats().damage;
        baseStats.speed = activeForm->getStats().speed;
        baseStats.defense = activeForm->getStats().defense;

        if (animator) {
            animator->setCharacterKey(getCharacter().getName() + "_" + activeForm->getVisualKey());
        }

        SoundManager::getInstance().playSound("switch");
        switchCooldownTimer = SWITCH_COOLDOWN_DURATION;
        return true;
    }

    return false;
}

void Player::gainMomentum(float amount, FormType form) {
    auto it = formMomentum.find(form);
    if (it != formMomentum.end()) {
        it->second += amount;
        it->second = std::clamp(it->second, 0.0f, MAX_MOMENTUM);
    }
}

void Player::setMomentum(float amount, FormType form) {
    auto it = formMomentum.find(form);
    if (it != formMomentum.end()) {
        it->second = std::clamp(amount, 0.0f, MAX_MOMENTUM);
    }
}

void Player::triggerSpecial(int abilityIndex, class Chamber& chamber) {
    if (!activeForm) return;

    FormType currentForm = activeForm->getFormType();
    float& momentum = formMomentum[currentForm];

    if (abilityIndex == 1 && momentum >= special1Threshold) {
        auto specialState = activeForm->createSpecialState(1);
        if (specialState) {
            stateMachine.enterTemporaryState(std::move(specialState), *this);
        }
        momentum = 0.0f;
    } else if (abilityIndex == 2 && momentum >= MAX_MOMENTUM) {
        auto specialState = activeForm->createSpecialState(2);
        if (specialState) {
            stateMachine.enterTemporaryState(std::move(specialState), *this);
        }
        momentum = 0.0f;
    }
}

void Player::attack(sf::Vector2f targetDir, class Chamber& chamber) {
    if (!canAct() || animController.isAttacking() || hasPendingAttack) return;

    if (Math::length(targetDir) > 0.01f) {
        movementController.setFacingFromVector(targetDir);
    }

    animController.triggerAttackAnim(*this, movementController.getFacingString());

    // Dispatch attack logic via active combat state immediately
    if (stateMachine.getActiveState()) {
        stateMachine.getActiveState()->onAttack(*this, targetDir, chamber);
    }
}


float Player::getMomentum(FormType form) const {
    auto it = formMomentum.find(form);
    if (it != formMomentum.end()) {
        return it->second;
    }
    return 0.0f;
}

void Player::setSwitchCooldownEnabled(bool enabled) {
    isSwitchCooldownEnabled = enabled;
}

void Player::setFacingFromVector(const sf::Vector2f& dir) {
    movementController.setFacingFromVector(dir);
}

void Player::setInMidChamber(bool value) {
    inMidChamber = value;
    setSwitchCooldownEnabled(!value);
}

bool Player::getInMidChamber() const {
    return inMidChamber;
}

float Player::getSwitchCooldownTimer() const {
    return switchCooldownTimer;
}

FormType Player::getActiveFormType() const {
    return activeForm ? activeForm->getFormType() : FormType::WRAITHBLADE;
}

const PlayableCharacter& Player::getCharacter() const {
    return *character;
}

void Player::setSpecial1Threshold(float threshold) {
    special1Threshold = threshold;
}

float Player::getSpecial1Threshold() const {
    return special1Threshold;
}

void Player::setDamage(float dmg) {
    baseStats.damage = dmg;
    for (auto& [type, form] : forms) {
        if (form) {
            form->setDamage(dmg);
        }
    }
}

float Player::getDamage() const {
    return getEffectiveStats().damage;
}

PlayerCombatStateMachine& Player::getStateMachine() {
    return stateMachine;
}

Stats Player::getEffectiveStats() const {
    Stats stats = baseStats;
    if (stateMachine.getActiveState())
        stats = stateMachine.getActiveState()->getStats();
    // Shared HP pool is tracked on the character's baseStats.hp
    stats.hp = baseStats.hp;
    // Apply status effects modifiers on top of base/state stats
    for (const auto& effect : statusEffects) {
        stats = effect->getStatModifier().applyTo(stats);
    }
    return stats;
}

bool Player::getIsFacingRight() const {
    return movementController.getIsFacingRight();
}

std::string Player::getFacingString() const {
    return movementController.getFacingString();
}


UI::PlayerHealthBar* Player::getPlayerHealthBar() {
    return dynamic_cast<UI::PlayerHealthBar*>(healthBar.get());
}

const UI::PlayerHealthBar* Player::getPlayerHealthBar() const {
    return dynamic_cast<const UI::PlayerHealthBar*>(healthBar.get());
}

void Player::onWallCollision() {
    Character::onWallCollision();
    // Delegate to movement controller so auto-glide stops at the wall
    movementController.onWallCollision();
}

PlayerForm* Player::getActiveForm() const {
    return activeForm;
}

bool Player::isAnimationFinished() const {
    return animator && animator->isCurrentAnimationFinished();
}

