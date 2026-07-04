#include "slowed-effect.hpp"

SlowedEffect::SlowedEffect() :
    StatusEffect(10.0f, { .speedMultiplier = 0.7f }) {}

SlowedEffect::SlowedEffect (float duration) :
    StatusEffect(duration, { .speedMultiplier = 0.7f }) {}

void SlowedEffect::apply(Character &character) {
    // No-op - speed is handled dynamically in getEffectiveStats()
}

void SlowedEffect::remove(Character &character) {
    // No-op - speed is handled dynamically in getEffectiveStats()
}

bool SlowedEffect::update(float dt, Character &character) {
    return StatusEffect::update(dt, character);
}