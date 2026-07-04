#ifndef PARALYZED_EFFECT_HPP
#define PARALYZED_EFFECT_HPP

#include "status-effect.hpp"
#include "../character.hpp"

class ParalyzedEffect : public StatusEffect {
public:
    ParalyzedEffect();
    ParalyzedEffect(float duration);

    void apply(Character& character) override;
    void remove(Character& character) override;
};

#endif // PARALYZED_EFFECT_HPP
