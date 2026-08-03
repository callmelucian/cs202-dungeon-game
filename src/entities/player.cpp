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
    
    // Section 7.1: Grid coordinate alignment
    float cellSize = settings.getCellSize();
    float ox = settings.getGridOffsetX();
    float oy = settings.getGridOffsetY();
    sf::Vector2f pos = getPosition();

    if (dir.x == 0.f && dir.y == 0.f) {
        int col = static_cast<int>(std::floor((pos.x - ox) / cellSize));
        int row = static_cast<int>(std::floor((pos.y - oy) / cellSize));
        sf::Vector2f targetPos(ox + (col + 0.5f) * cellSize, oy + (row + 0.5f) * cellSize);
        sf::Vector2f diff = targetPos - pos;
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        if (dist > 0.01f && dist < cellSize * 0.5f) {
            setPosition(pos + diff * std::min(1.0f, deltaTime * 15.0f));
        }
    } else {
        // Active movement axis alignment in movement direction
        if (dir.x != 0.f && dir.y == 0.f) {
            // Moving horizontally: look ahead along X and align Y center
            float lookX = pos.x + dir.x * (cellSize * 0.4f);
            int targetRow = static_cast<int>(std::floor((pos.y - oy) / cellSize));
            float targetY = oy + (targetRow + 0.5f) * cellSize;
            float diffY = targetY - pos.y;
            if (std::abs(diffY) > 0.01f && std::abs(diffY) < cellSize * 0.45f) {
                pos.y += diffY * std::min(1.0f, deltaTime * 20.0f);
                setPosition(pos);
            }
        } else if (dir.y != 0.f && dir.x == 0.f) {
            // Moving vertically: look ahead along Y and align X center
            float lookY = pos.y + dir.y * (cellSize * 0.4f);
            int targetCol = static_cast<int>(std::floor((pos.x - ox) / cellSize));
            float targetX = ox + (targetCol + 0.5f) * cellSize;
            float diffX = targetX - pos.x;
            if (std::abs(diffX) > 0.01f && std::abs(diffX) < cellSize * 0.45f) {
                pos.x += diffX * std::min(1.0f, deltaTime * 20.0f);
                setPosition(pos);
            }
        }
    }

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

    // 6. Update player momentum on healthBar (health update & drawing handled by Character)
    if (healthBar) {
        healthBar->setMomentum(getMomentum(getActiveFormType()), special1Threshold);
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

    SoundManager::getInstance().playSound("switch");
    switchCooldownTimer = SWITCH_COOLDOWN_DURATION;
    return true;
}

void Player::gainMomentum(float amount, FormType form) {
    auto it = formMomentum.find(form);
    if (it != formMomentum.end()) {
        it->second += amount;
        it->second = std::clamp(it->second, 0.0f, MAX_MOMENTUM);
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

UI::PlayerHealthBar* Player::getPlayerHealthBar() {
    return dynamic_cast<UI::PlayerHealthBar*>(healthBar.get());
}

const UI::PlayerHealthBar* Player::getPlayerHealthBar() const {
    return dynamic_cast<const UI::PlayerHealthBar*>(healthBar.get());
}
