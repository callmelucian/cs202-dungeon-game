#ifndef BURNED_EFFECT_HPP
#define BURNED_EFFECT_HPP

#include "status-effect.hpp"
#include "../character.hpp"

class BurnedEffect : public StatusEffect {
public:
    BurnedEffect (float damagePerTick);
    BurnedEffect (float damagePerTick, float duration);

    void apply (Character &character) override;
    void remove (Character &character) override;
    bool update (float dt, Character &character) override;
    std::string getName() const override { return "Burned"; }
    sf::Color getColor() const override { return sf::Color(255, 120, 50); }
    std::string getDisplayName() const override { return "BURNED"; }

private:
    float damagePerTick, countDown;
};

#endif // BURNED_EFFECT_HPP