#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <string>
#include "effects/status-effect.hpp"
#include "stats.hpp"
#include "../ui/widgets/health-bar.hpp"

class CharacterObserver;
class CharacterAnimator;

class Character {
public:
    Character(const std::string& characterKey);
    virtual ~Character();

    virtual void update(float deltaTime) = 0;
    virtual void draw(sf::RenderWindow &window) const;
    virtual void takeDamage(float rawAmount, bool isCritical = false);
    virtual void heal(float amount);
    void applyStatusEffect(std::unique_ptr<StatusEffect> effect);
    void clearNegativeStatusEffects();

    void addObserver(CharacterObserver* observer);
    void removeObserver(CharacterObserver* observer);

    sf::Vector2f getPosition() const;
    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getVelocity() const;
    void setVelocity(const sf::Vector2f& vel);
    sf::Vector2f getKnockbackVelocity() const;
    void setKnockbackVelocity(const sf::Vector2f& kvel);
    sf::Vector2f getEffectiveVelocity() const;
    void applyKnockback(const sf::Vector2f& direction, float magnitude);
    virtual void onWallCollision();
    float getHp() const;
    void setHp(float hp);
    void setMaxHp(float maxHp);
    float getSpeed() const;
    void setSpeed(float speed);
    bool canAct() const;
    virtual sf::FloatRect getBounds() const;
    virtual Stats getEffectiveStats() const;
    bool isAlive() const;
    bool isSlowed() const;
    bool hasStatusEffect(const std::string& name) const;
    const std::vector<std::unique_ptr<StatusEffect>>& getStatusEffects() const { return statusEffects; }

    virtual std::string getDisplayName() const;
    void setDisplayName(const std::string& name);
    void setCharacterKey(const std::string& key);

    UI::HealthBar* getHealthBar();
    const UI::HealthBar* getHealthBar() const;
    void setHealthBar(std::unique_ptr<UI::HealthBar> bar);

    void setTint(const sf::Color& color);
    sf::Color getTint() const;
    bool isFrozen() const;
    void setFrozen(bool frozen);

protected:
    void notifyStateChanged(std::string visualKey);
    float calculateMitigatedDamage(float rawAmount);
    void tickStatusEffects(float dt);

    std::string characterKey;
    std::string displayName;
    sf::Vector2f position, velocity;
    sf::Vector2f knockbackVelocity;
    Stats baseStats;
    std::vector<std::unique_ptr<StatusEffect>> statusEffects;
    std::vector<std::unique_ptr<StatusEffect>> pendingStatusEffects;
    std::vector<CharacterObserver*> observers;
    std::unique_ptr<CharacterAnimator> animator;
    std::unique_ptr<UI::HealthBar> healthBar;
    bool isFrozenState = false;
};

#endif // CHARACTER_HPP