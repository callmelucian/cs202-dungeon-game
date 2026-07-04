#include "paralyzed-effect.hpp"

ParalyzedEffect::ParalyzedEffect()
    : StatusEffect(10.0f, { .newMissChance = 0.4f }) {}

ParalyzedEffect::ParalyzedEffect (float duration)
    : StatusEffect(duration, { .newMissChance = 0.4f }) {}

void ParalyzedEffect::apply(Character& character) {
    // No attributes are directly modified upon application
}

void ParalyzedEffect::remove(Character& character) {
    // No cleanup required upon removal
}
