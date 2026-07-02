#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "status-effect.hpp"

class Character {
public:
    Character();
    virtual ~Character() = default;

    virtual void update (float deltaTime) = 0;
    virtual void draw (sf::RenderWindow &window) = 0;
    void takeDamage (float amount);
    void applyStatusEffect (std::unique_ptr<StatusEffect> effect);

    sf::Vector2f getPosition() const;
    float getHp() const;
    float getSpeed() const;
    void setSpeed(float speed);

protected:
    sf::Vector2f position, velocity;
    float hp, maxHp, baseDamage, moveSpeed, attackRange, attackRate;
    std::vector<std::unique_ptr<StatusEffect>> statusEffects;
};

#endif // CHARACTER_HPP