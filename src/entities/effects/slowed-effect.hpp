#ifndef SLOWED_EFFECT_HPP
#define SLOWED_EFFECT_HPP

#include "../status-effect.hpp"
#include "../character.hpp"

class SlowedEffect : public StatusEffect {
public:
    SlowedEffect();
    SlowedEffect(float duration);
    
    void apply (Character &character) override;
    void remove (Character &character) override;

private:
    float speedReductionPercent, originalSpeed;
};

#endif // SLOWED_EFFECT_HPP