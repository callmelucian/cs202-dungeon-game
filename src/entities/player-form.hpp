#ifndef PLAYER_FORM_HPP
#define PLAYER_FORM_HPP

#include <SFML/System/Vector2.hpp>
#include "../core/enums.hpp"

class Player;
class Chamber;

struct FormStats {
    float baseSpeed;
    float maxHpContribution;
    float baseDamage;
    float attackRange;
    float attackRate;
};

class PlayerForm {
public:
    PlayerForm(FormType formType, float baseSpeed, float maxHpContribution, float baseDamage, float attackRange, float attackRate);
    virtual ~PlayerForm() = default;

    virtual void attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) = 0;
    virtual void triggerSpecial1(Player& player, Chamber& chamber) = 0;
    virtual void triggerSpecial2(Player& player, Chamber& chamber) = 0;

    FormStats getStats() const;
    FormType getFormType() const;

protected:
    FormType formType;
    float baseSpeed;
    float maxHpContribution;
    float baseDamage;
    float attackRange;
    float attackRate;
};

#endif // PLAYER_FORM_HPP
