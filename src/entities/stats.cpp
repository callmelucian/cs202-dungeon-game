#include "stats.hpp"

Stats StatModifier::applyTo (const Stats &base) const {
    Stats result = base;
    result.hp *= hpMultiplier;
    result.maxHp *= hpMultiplier;
    result.damage *= damageMultiplier;
    result.speed *= speedMultiplier;
    result.missChance = newMissChance;
    return result;
}