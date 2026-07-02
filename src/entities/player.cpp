#include "player.hpp"
#include "forms/wraithblade-form.hpp"
#include "forms/voidcaster-form.hpp"
#include "forms/ironshell-form.hpp"
#include <algorithm>

Player::Player()
    : wraithbladeMomentum(0.0f),
      voidcasterMomentum(0.0f),
      ironshellMomentum(0.0f),
      switchCooldownTimer(0.0f),
      isSwitchCooldownEnabled(true)
{
    // Start in Wraithblade form by default
    activeForm = std::make_unique<WraithbladeForm>();
    FormStats stats = activeForm->getStats();
    
    // Sync initial stats to Character base fields
    hp = stats.maxHpContribution;
    maxHp = stats.maxHpContribution;
    moveSpeed = stats.baseSpeed;
    baseDamage = stats.baseDamage;
    attackRange = stats.attackRange;
    attackRate = stats.attackRate;
}

void Player::update(float deltaTime) {
    // 1. Update form switch cooldown
    if (switchCooldownTimer > 0.0f) {
        switchCooldownTimer -= deltaTime;
        if (switchCooldownTimer < 0.0f) {
            switchCooldownTimer = 0.0f;
        }
    }

    // 2. Base Character update (updates active status effects)
    Character::update(deltaTime);

    // 3. Ironshell aura application stub
    if (activeForm && activeForm->getFormType() == FormType::IRONSHELL) {
        // In actual implementation with Chamber:
        // for (auto& enemy : chamber.getEnemies()) {
        //     if (distance(getPosition(), enemy->getPosition()) <= 4.0f) {
        //         enemy->applyStatusEffect(std::make_unique<SlowedEffect>());
        //     }
        // }
    }
}

void Player::draw(sf::RenderWindow &window) {
    // Stub draw implementation
}

void Player::takeDamage(float amount) {
    // Apply damage to HP
    Character::takeDamage(amount);

    // Gain momentum on damage taken according to the active form's rules
    if (activeForm) {
        FormType currentForm = activeForm->getFormType();
        if (currentForm == FormType::WRAITHBLADE) {
            gainMomentum(amount * 0.4f, FormType::WRAITHBLADE);
        } else if (currentForm == FormType::VOIDCASTER) {
            gainMomentum(amount * 0.4f, FormType::VOIDCASTER);
        } else if (currentForm == FormType::IRONSHELL) {
            gainMomentum(amount * 1.2f, FormType::IRONSHELL);
        }
    }
}

void Player::switchForm(FormType newForm) {
    if (!activeForm) return;
    if (activeForm->getFormType() == newForm) return;

    // Check switch cooldown
    if (isSwitchCooldownEnabled && switchCooldownTimer > 0.0f) {
        return;
    }

    // 1. Reset current form's momentum to 0 upon switching away
    getFormMomentumRef(activeForm->getFormType()) = 0.0f;

    // 2. Store current HP percentage to apply conversion formula
    float currentMaxHp = activeForm->getStats().maxHpContribution;

    // 3. Swap form strategy
    if (newForm == FormType::WRAITHBLADE) {
        activeForm = std::make_unique<WraithbladeForm>();
    } else if (newForm == FormType::VOIDCASTER) {
        activeForm = std::make_unique<VoidcasterForm>();
    } else if (newForm == FormType::IRONSHELL) {
        activeForm = std::make_unique<IronshellForm>();
    }

    // 4. Update stats and apply HP conversion: new_HP = (current_HP / current_form_max_HP) * new_form_max_HP
    FormStats stats = activeForm->getStats();
    float newMaxHp = stats.maxHpContribution;
    hp = (hp / currentMaxHp) * newMaxHp;
    maxHp = newMaxHp;

    moveSpeed = stats.baseSpeed;
    baseDamage = stats.baseDamage;
    attackRange = stats.attackRange;
    attackRate = stats.attackRate;

    // 5. Trigger switch cooldown (4.0s)
    switchCooldownTimer = 4.0f;
}

void Player::gainMomentum(float amount, FormType form) {
    float& momentum = getFormMomentumRef(form);
    momentum += amount;
    if (momentum > 100.0f) {
        momentum = 100.0f;
    } else if (momentum < 0.0f) {
        momentum = 0.0f;
    }
}

void Player::triggerSpecial(int abilityIndex, class Chamber& chamber) {
    if (!activeForm) return;

    FormType currentForm = activeForm->getFormType();
    float& momentum = getFormMomentumRef(currentForm);

    if (abilityIndex == 1 && momentum >= 50.0f) {
        activeForm->triggerSpecial1(*this, chamber);
        momentum = 0.0f; // Reset to 0 upon use
    } else if (abilityIndex == 2 && momentum >= 100.0f) {
        activeForm->triggerSpecial2(*this, chamber);
        momentum = 0.0f; // Reset to 0 upon use
    }
}

float& Player::getFormMomentumRef(FormType form) {
    if (form == FormType::WRAITHBLADE) {
        return wraithbladeMomentum;
    } else if (form == FormType::VOIDCASTER) {
        return voidcasterMomentum;
    } else {
        return ironshellMomentum;
    }
}

float Player::getMomentum(FormType form) const {
    if (form == FormType::WRAITHBLADE) {
        return wraithbladeMomentum;
    } else if (form == FormType::VOIDCASTER) {
        return voidcasterMomentum;
    } else {
        return ironshellMomentum;
    }
}

void Player::setSwitchCooldownEnabled(bool enabled) {
    isSwitchCooldownEnabled = enabled;
}

float Player::getSwitchCooldownTimer() const {
    return switchCooldownTimer;
}
