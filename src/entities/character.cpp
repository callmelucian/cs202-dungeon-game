#include "character.hpp"
#include <algorithm>

Character::Character()
    : position(0.0f, 0.0f),
      velocity(0.0f, 0.0f),
      hp(100.0f),
      maxHp(100.0f),
      baseDamage(10.0f),
      moveSpeed(100.0f),
      attackRange(50.0f),
      attackRate(1.0f) {}

void Character::update(float deltaTime) {
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

void Character::takeDamage(float amount) {
    hp -= amount;
    if (hp < 0.0f) {
        hp = 0.0f;
    }
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

sf::Vector2f Character::getPosition() const {
    return position;
}

float Character::getHp() const {
    return hp;
}

float Character::getSpeed() const {
    return moveSpeed;
}

void Character::setSpeed(float speed) {
    moveSpeed = speed;
}
