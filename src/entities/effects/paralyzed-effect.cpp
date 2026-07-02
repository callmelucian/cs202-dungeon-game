#include "paralyzed-effect.hpp"

ParalyzedEffect::ParalyzedEffect()
    : StatusEffect(10.0f), missChance(0.40f) {}

ParalyzedEffect::ParalyzedEffect(float duration)
    : StatusEffect(duration), missChance(0.40f) {}

void ParalyzedEffect::apply(Character& character) {
    // No attributes are directly modified upon application
}

void ParalyzedEffect::remove(Character& character) {
    // No cleanup required upon removal
}

float ParalyzedEffect::getMissChance() const {
    return missChance;
}
