#ifndef CHARACTER_ANIMATOR_HPP
#define CHARACTER_ANIMATOR_HPP

#include "../character.hpp"

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
    CharacterAnimator(const std::string& characterKey);
    void onStateChanged(const Character& character, std::string animationKey) override;
    void onDamaged(const Character& character, float amount) override;
    void onDefeated(const Character& character) override;
    
    void update(float dt, float speedMultiplier = 1.0f);
    void draw(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2f size = sf::Vector2f(0.f, 0.f)) const;
    void setCharacterKey(const std::string& newKey);
    bool isCurrentAnimationFinished() const;
    bool hasSprite() const { return sprite != nullptr; }

private:
    std::unique_ptr<sf::Sprite> sprite;
    std::string characterKey;
    std::string currentAnimationKey;
    float hitFlashTimer, currentAnimationElapsed;
};

#endif // CHARACTER_ANIMATOR_HPP