#ifndef STATS_HPP
#define STATS_HPP

struct Stats {
    float hp = 0.0f;
    float maxHp = 0.0f;
    float damage = 0.0f;
    float speed = 0.0f;
    float defense = 0.0f;
    float missChance = 0.0;
};

struct StatModifier {
    float hpMultiplier = 1.0f;
    float damageMultiplier = 1.0f;
    float speedMultiplier = 1.0f;
    float newMissChance = 0.0f;

    Stats applyTo(const Stats& base) const;
};

#endif // STATS_HPP
