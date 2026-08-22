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
    std::string getName() const override { return "Paralyzed"; }
    sf::Color getColor() const override { return sf::Color(255, 230, 80); }
    std::string getDisplayName() const override { return "PARALYZED"; }
};

#endif // PARALYZED_EFFECT_HPP
