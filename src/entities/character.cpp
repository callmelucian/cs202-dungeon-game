#include "character.hpp"
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include "effects/paralyzed-effect.hpp"
#include "../utils/math-utility.hpp"

// ---- CharacterAnimator implementation ----
CharacterAnimator::CharacterAnimator() : hitFlashTimer(0.0f) {}

void CharacterAnimator::onStateChanged(const Character& character, std::string visualKey) {
    currentAnimKey = visualKey;
}

void CharacterAnimator::onDamaged(const Character& character, float amount) {
    hitFlashTimer = 0.2f;
}

void CharacterAnimator::onDefeated(const Character& character) {
    // Play defeat animations / effects
}

void CharacterAnimator::update(float dt) {
    if (hitFlashTimer > 0.0f) {
        hitFlashTimer -= dt;
    }
}

void CharacterAnimator::draw(sf::RenderWindow& window) {
    // Draw logic placeholder using sprite
}


// ---- Character implementation ----
Character::Character()
    : position(0.0f, 0.0f),
      velocity(0.0f, 0.0f),
      animator(std::make_unique<CharacterAnimator>())
{
    baseStats.hp = 100.0f;
    baseStats.maxHp = 100.0f;
    baseStats.damage = 10.0f;
    baseStats.speed = 100.0f;
    baseStats.defense = 0.0f;
}

void Character::update(float deltaTime) {
    if (animator) {
        animator->update(deltaTime);
    }

    // Update active status effects and remove finished ones
    for (auto it = statusEffects.begin(); it != statusEffects.end(); ) {
        bool active = (*it)->update(deltaTime, *this);
        if (!active) {
            (*it)->remove(*this);
            it = statusEffects.erase(it);
        }
        else ++it;
    }
}

void Character::draw(sf::RenderWindow &window) {
    if (animator) {
        animator->draw(window);
    }
}

void Character::takeDamage(float rawAmount) {
    float mitigated = calculateMitigatedDamage(rawAmount);
    baseStats.hp -= mitigated;
    if (baseStats.hp < 0.0f) {
        baseStats.hp = 0.0f;
    }

    for (auto observer : observers) {
        observer->onDamaged(*this, mitigated);
        if (!isAlive()) {
            observer->onDefeated(*this);
        }
    }
}

float Character::calculateMitigatedDamage(float rawAmount) {
    float defense = getEffectiveStats().defense;
    float mitigated = rawAmount * 100.0f / (100.0f + defense);
    return std::max(1.0f, std::round(mitigated));
}

void Character::applyStatusEffect(std::unique_ptr<StatusEffect> effect) {
    if (!effect) return;

    // Check if an effect of the same type is already applied
    for (auto& activeEffect : statusEffects) {
        if (typeid(*activeEffect) == typeid(*effect)) {
            activeEffect->refresh(*effect);
            return;
        }
    }

    // Apply the effect and store it
    effect->apply(*this);
    statusEffects.push_back(std::move(effect));
}

void Character::addObserver(CharacterObserver* observer) {
    if (observer && std::find(observers.begin(), observers.end(), observer) == observers.end()) {
        observers.push_back(observer);
    }
}

void Character::removeObserver(CharacterObserver* observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}

void Character::notifyStateChanged(std::string visualKey) {
    for (auto observer : observers) {
        observer->onStateChanged(*this, visualKey);
    }
}

sf::Vector2f Character::getPosition() const {
    return position;
}

float Character::getHp() const {
    return baseStats.hp;
}

float Character::getSpeed() const {
    return getEffectiveStats().speed;
}

void Character::setSpeed(float speed) {
    baseStats.speed = speed;
}

bool Character::canAct() {
    return !Math::decide(getEffectiveStats().missChance);
}

Stats Character::getEffectiveStats() const {
    Stats stats = baseStats;
    for (const auto& effect : statusEffects) {
        stats = effect->getStatModifier().applyTo(stats);
    }
    return stats;
}

bool Character::isAlive() const {
    return baseStats.hp > 0.0f;
}
