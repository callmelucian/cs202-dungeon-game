#ifndef SLOWED_EFFECT_HPP
#define SLOWED_EFFECT_HPP

#include "status-effect.hpp"
#include "../character.hpp"

class SlowedEffect : public StatusEffect {
public:
    SlowedEffect();
    SlowedEffect(float duration);
    
    void apply (Character &character) override;
    void remove (Character &character) override;
    bool update(float dt, Character &character) override;
    std::string getName() const override { return "Slowed"; }
    sf::Color getColor() const override { return sf::Color(100, 200, 255); }
    std::string getDisplayName() const override { return "SLOWED"; }
};

#endif // SLOWED_EFFECT_HPP