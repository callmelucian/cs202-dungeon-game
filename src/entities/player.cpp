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

Player::Player(PlayableCharacter& character)
    : Character(character.getName()),
      character(&character),
      activeForm(nullptr),
      switchCooldownTimer(0.0f),
      isSwitchCooldownEnabled(true),
      isFacingRight(true),
      isAttacking(false)
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

void Player::update(float deltaTime) {
    // 1. Handle real-time movement input
    SettingManager& settings = SettingManager::getInstance();
    sf::Vector2f dir(0.f, 0.f);

    if (sf::Keyboard::isKeyPressed(settings.getKeyBinding("MoveUp"))) dir.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(settings.getKeyBinding("MoveDown"))) dir.y += 1.f;
    if (sf::Keyboard::isKeyPressed(settings.getKeyBinding("MoveLeft"))) dir.x -= 1.f, isFacingRight = false;
    if (sf::Keyboard::isKeyPressed(settings.getKeyBinding("MoveRight"))) dir.x += 1.f, isFacingRight = true;

    if (isAttacking) {
        if (animator && animator->isCurrentAnimationFinished()) {
            isAttacking = false;
        }
    }

    // Normalize diagonal movement speed
    std::string direction = (isFacingRight ? "right" : "left");
    if (!isAttacking) {
        if (dir.x != 0.f || dir.y != 0.f) {
            float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            dir.x /= length;
            dir.y /= length;
            notifyStateChanged(std::string("run-facing-") + direction);
        }
        else notifyStateChanged(std::string("idle-facing-") + direction);
    } else {
        if (dir.x != 0.f || dir.y != 0.f) {
            float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            dir.x /= length;
            dir.y /= length;
        }
    }

    // Scale movement based on cell size (originally 60.f for a 100.f cell size)
    const float SPEED_TO_PIXELS = SettingManager::getInstance().getCellSize() * SettingManager::getInstance().getSpeedMultiplier();
    setVelocity(dir * getSpeed() * SPEED_TO_PIXELS);

    // 2. Update cooldown
    if (switchCooldownTimer > 0.0f) {
        switchCooldownTimer -= deltaTime;
        if (switchCooldownTimer < 0.0f) {
            switchCooldownTimer = 0.0f;
        }
    }

    // 3. State machine update (updates active state)
    stateMachine.update(*this, deltaTime);

    // 4. Character base update (updates status effects)
    Character::update(deltaTime);

    // 5. Apply SlowAura if Ironshell
    if (getActiveFormType() == FormType::IRONSHELL && currentChamber) {
        auto enemies = currentChamber->getEnemiesRaw();
        applySlowAura(enemies);
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

void Player::draw(sf::RenderWindow &window) const {
    // Let the base animator draw the sprite if implemented
    Character::draw(window);

    // Fallback: draw the bounding box as a green square
    sf::FloatRect bounds = getBounds();
    animator->draw(window, getPosition(), bounds.size * SettingManager::getInstance().getSpriteMultiplier());
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

    // 3. Gain momentum based on actual (post-mitigation) HP lost
    if (actualHpLost > 0.0f) {
        FormType currentForm = getActiveFormType();
        if (currentForm == FormType::WRAITHBLADE) {
            gainMomentum(actualHpLost * 0.4f, FormType::WRAITHBLADE);
        } else if (currentForm == FormType::VOIDCASTER) {
            gainMomentum(actualHpLost * 0.4f, FormType::VOIDCASTER);
        } else if (currentForm == FormType::IRONSHELL) {
            gainMomentum(actualHpLost * 1.2f, FormType::IRONSHELL);
        }
    }
}

bool Player::switchForm(FormType newForm) {
    if (activeForm && activeForm->getFormType() == newForm) return false;
    if (isSwitchCooldownEnabled && switchCooldownTimer > 0.0f) return false;

    // Reset momentum of form we are switching away from
    if (activeForm) formMomentum[activeForm->getFormType()] = 0.0f;

    auto it = forms.find(newForm);
    if (it != forms.end()) {
        activeForm = it->second.get();
        stateMachine.setBaseState(activeForm, *this);
    }

    baseStats.damage = activeForm->getStats().damage;
    baseStats.speed = activeForm->getStats().speed;
    baseStats.defense = activeForm->getStats().defense;

    if (animator) {
        animator->setCharacterKey(getCharacter().getName() + "_" + activeForm->getVisualKey());
    }

    switchCooldownTimer = 4.0f;
    return true;
}

void Player::gainMomentum(float amount, FormType form) {
    auto it = formMomentum.find(form);
    if (it != formMomentum.end()) {
        it->second += amount;
        it->second = std::clamp(it->second, 0.0f, 100.0f);
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
    } else if (abilityIndex == 2 && momentum >= 100.0f) {
        auto specialState = activeForm->createSpecialState(2);
        if (specialState) {
            stateMachine.enterTemporaryState(std::move(specialState), *this);
        }
        momentum = 0.0f;
    }
}

void Player::attack(sf::Vector2f targetDir, class Chamber& chamber) {
    // check if it's ready to attack
    if (!canAct()) {
        // possibly render a paralyzing animation
        return;
    }

    // We only use left and right sprite directions in this game
    std::string attackDir = isFacingRight ? "right" : "left";

    std::string animName;
    FormType type = getActiveFormType();
    if (type == FormType::VOIDCASTER) {
        animName = "shoot-facing-";
    } else if (type == FormType::WRAITHBLADE) {
        animName = "backslash-facing-";
    } else {
        animName = "slash-facing-";
    }
    
    isAttacking = true;
    notifyStateChanged(animName + attackDir);

    // attack
    if (stateMachine.getActiveState())
        stateMachine.getActiveState()->onAttack(*this, targetDir, chamber);
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
    return isFacingRight;
}
