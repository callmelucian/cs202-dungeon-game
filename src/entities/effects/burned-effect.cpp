#include "burned-effect.hpp"

BurnedEffect::BurnedEffect (float damagePerTick) :
    StatusEffect(10.f), damagePerTick(damagePerTick), countDown(1.0) {}

BurnedEffect::BurnedEffect (float damagerPerTick, float duration) :
    StatusEffect(duration), damagePerTick(damagePerTick), countDown(1.0) {}

void BurnedEffect::apply (Character &character) {
    // register visual effects for burning
}

void BurnedEffect::remove (Character &character) {
    // remove visual effects for burning
}

bool BurnedEffect::update (float dt, Character &character) {
    if ((countDown -= dt) <= 0) {
        countDown = 1.0;
        character.takeDamage(damagePerTick);
    }
    return StatusEffect::update(dt, character);
}