#include "slowed-effect.hpp"

SlowedEffect::SlowedEffect()
    : StatusEffect(10.0f), speedReductionPercent(0.30f), originalSpeed(0.0f) {}

SlowedEffect::SlowedEffect(float duration)
    : StatusEffect(duration), speedReductionPercent(0.30f), originalSpeed(0.0f) {}

void SlowedEffect::apply(Character &character) {
    originalSpeed = character.getSpeed();
    character.setSpeed(originalSpeed * (1.0f - speedReductionPercent));
}

void SlowedEffect::remove(Character &character) {
    character.setSpeed(originalSpeed);
}
