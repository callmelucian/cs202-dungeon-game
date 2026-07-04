#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <string>
#include "effects/status-effect.hpp"
#include "stats.hpp"

class Character;

class CharacterObserver {
public:
    virtual ~CharacterObserver() = default;
    virtual void onStateChanged(const Character& character, std::string visualKey) = 0;
    virtual void onDamaged(const Character& character, float amount) = 0;
    virtual void onDefeated(const Character& character) = 0;
};

class CharacterAnimator : public CharacterObserver {
public:
    CharacterAnimator();
    void onStateChanged(const Character& character, std::string visualKey) override;
    void onDamaged(const Character& character, float amount) override;
    void onDefeated(const Character& character) override;
    
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    std::unique_ptr<sf::Sprite> sprite;
    std::string currentAnimKey;
    float hitFlashTimer;
};

class Character {
public:
    Character();
    virtual ~Character() = default;

    virtual void update(float deltaTime) = 0;
    virtual void draw(sf::RenderWindow &window) = 0;
    
    virtual void takeDamage(float rawAmount);
    void applyStatusEffect(std::unique_ptr<StatusEffect> effect);

    void addObserver(CharacterObserver* observer);
    void removeObserver(CharacterObserver* observer);

    sf::Vector2f getPosition() const;
    float getHp() const;
    float getSpeed() const;
    void setSpeed(float speed);
    bool canAct();

    virtual Stats getEffectiveStats() const;
    bool isAlive() const;

protected:
    void notifyStateChanged(std::string visualKey);
    float calculateMitigatedDamage(float rawAmount);

    sf::Vector2f position, velocity;
    Stats baseStats;
    std::vector<std::unique_ptr<StatusEffect>> statusEffects;
    std::vector<CharacterObserver*> observers;
    std::unique_ptr<CharacterAnimator> animator;
};

#endif // CHARACTER_HPP