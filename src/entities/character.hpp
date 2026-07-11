#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <string>
#include "effects/status-effect.hpp"
#include "stats.hpp"

class CharacterObserver;
class CharacterAnimator;

class Character {
public:
    Character(const std::string& characterKey);
    virtual ~Character();

    virtual void update(float deltaTime) = 0;
    virtual void draw(sf::RenderWindow &window) const = 0;
    
    virtual void takeDamage(float rawAmount);
    void applyStatusEffect(std::unique_ptr<StatusEffect> effect);

    void addObserver(CharacterObserver* observer);
    void removeObserver(CharacterObserver* observer);

    sf::Vector2f getPosition() const;
    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getVelocity() const;
    void setVelocity(const sf::Vector2f& vel);
    float getHp() const;
    float getSpeed() const;
    void setSpeed(float speed);
    bool canAct() const;
    virtual sf::FloatRect getBounds() const;

    virtual Stats getEffectiveStats() const;
    bool isAlive() const;

protected:
    void notifyStateChanged(std::string visualKey);
    float calculateMitigatedDamage(float rawAmount);
    void tickStatusEffects(float dt);

    sf::Vector2f position, velocity;
    Stats baseStats;
    std::vector<std::unique_ptr<StatusEffect>> statusEffects;
    std::vector<std::unique_ptr<StatusEffect>> pendingStatusEffects;
    std::vector<CharacterObserver*> observers;
    std::unique_ptr<CharacterAnimator> animator;
};

#endif // CHARACTER_HPP