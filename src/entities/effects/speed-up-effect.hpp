#ifndef SPEED_UP_EFFECT_HPP
#define SPEED_UP_EFFECT_HPP

#include "status-effect.hpp"

class SpeedUpEffect : public StatusEffect {
public:
    SpeedUpEffect(float duration = 10.0f);

    void apply(Character& character) override;
    void remove(Character& character) override;
    bool update(float dt, Character& character) override;
    std::string getName() const override { return "SpeedUp"; }
    sf::Color getColor() const override { return sf::Color(255, 215, 0); }
    std::string getDisplayName() const override { return "SPEED UP"; }
};

#endif // SPEED_UP_EFFECT_HPP
