#ifndef CRITICAL_BOOST_EFFECT_HPP
#define CRITICAL_BOOST_EFFECT_HPP

#include "status-effect.hpp"

class CriticalBoostEffect : public StatusEffect {
public:
    CriticalBoostEffect(float duration = 7.0f);

    void apply(Character& character) override;
    void remove(Character& character) override;
    bool update(float dt, Character& character) override;
    std::string getName() const override { return "CriticalBoost"; }
    sf::Color getColor() const override { return sf::Color(255, 50, 50); }
    std::string getDisplayName() const override { return "CRIT BOOST"; }
};

#endif // CRITICAL_BOOST_EFFECT_HPP
