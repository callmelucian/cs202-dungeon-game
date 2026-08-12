#include "stats.hpp"
#include <algorithm>

Stats StatModifier::applyTo (const Stats &base) const {
    Stats result = base;
    result.maxHp *= hpMultiplier;
    result.damage *= damageMultiplier;
    result.speed *= speedMultiplier;
    result.missChance = std::max(result.missChance, newMissChance);
    return result;
}