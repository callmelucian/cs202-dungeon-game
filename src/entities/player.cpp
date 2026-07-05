#include "player.hpp"
#include "../utils/math-utility.hpp"
#include "../global-settings/setting-manager.hpp"
#include <algorithm>
#include <map>
#include <iostream>

Player::Player(PlayableCharacter& character)
    : character(&character),
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
    }
}

void Player::handleInput(const sf::Event& event) {
    SettingManager& settings = SettingManager::getInstance();

    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->scancode == settings.getKeyBinding("SwitchForm1")) {
            if (switchForm(FormType::WRAITHBLADE)) {
                std::cout << "Switched to Wraithblade! Speed: " << getSpeed() << std::endl;
            }
        } else if (keyEvent->scancode == settings.getKeyBinding("SwitchForm2")) {
            if (switchForm(FormType::VOIDCASTER)) {
                std::cout << "Switched to Voidcaster! Speed: " << getSpeed() << std::endl;
            }
        } else if (keyEvent->scancode == settings.getKeyBinding("SwitchForm3")) {
            if (switchForm(FormType::IRONSHELL)) {
                std::cout << "Switched to Ironshell! Speed: " << getSpeed() << std::endl;
            }
        }
    }
}

void Player::update(float deltaTime) {
    // 1. Handle real-time movement input
    SettingManager& settings = SettingManager::getInstance();
    sf::Vector2f dir(0.f, 0.f);

    if (sf::Keyboard::isKeyPressed(settings.getKeyBinding("MoveUp"))) dir.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(settings.getKeyBinding("MoveDown"))) dir.y += 1.f;
    if (sf::Keyboard::isKeyPressed(settings.getKeyBinding("MoveLeft"))) dir.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(settings.getKeyBinding("MoveRight"))) dir.x += 1.f;

    // Normalize diagonal movement speed
    if (dir.x != 0.f || dir.y != 0.f) {
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        dir.x /= length;
        dir.y /= length;
    }

    float screenScale = 60.f;
    setVelocity(dir * getSpeed() * screenScale);

    // 2. Update cooldown
    if (switchCooldownTimer > 0.0f) {
        switchCooldownTimer -= deltaTime;
        if (switchCooldownTimer < 0.0f) {
            switchCooldownTimer = 0.0f;
        }
    }

    // 2. State machine update (updates active state)
    stateMachine.update(*this, deltaTime);

    // 3. Character base update (updates status effects)
    Character::update(deltaTime);
}

void Player::draw(sf::RenderWindow &window) {
    // Let the base animator draw the sprite if implemented
    Character::draw(window);

    // Fallback: draw the bounding box as a green square
    sf::FloatRect bounds = getBounds();
    sf::RectangleShape pShape(bounds.size);
    pShape.setPosition(bounds.position);
    
    // Change color based on active form just for visual flair
    FormType currentForm = getActiveFormType();
    if (currentForm == FormType::WRAITHBLADE) {
        pShape.setFillColor(sf::Color(50, 180, 50)); // Green
    } else if (currentForm == FormType::VOIDCASTER) {
        pShape.setFillColor(sf::Color(150, 50, 180)); // Purple
    } else {
        pShape.setFillColor(sf::Color(180, 100, 50)); // Orange
    }
    
    window.draw(pShape);
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

    if (abilityIndex == 1 && momentum >= 50.0f) {
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
