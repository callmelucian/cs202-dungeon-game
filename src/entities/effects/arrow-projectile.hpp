#ifndef ARROW_PROJECTILE_HPP
#define ARROW_PROJECTILE_HPP

#include <SFML/Graphics.hpp>
#include <unordered_set>

class Chamber;
class Player;
class Enemy;

enum class ArrowHitMode {
    PIERCING,
    SINGLE_TARGET
};

class ArrowProjectile {
public:
    ArrowProjectile(sf::Vector2f startPos,
                    sf::Vector2f direction,
                    float maxDistance,
                    float speed = 900.0f,
                    ArrowHitMode hitMode = ArrowHitMode::PIERCING);

    void update(float dt, Chamber& chamber, Player& player);
    void draw(sf::RenderWindow& window) const;

    bool isActive() const;
    void deactivate();

    sf::Vector2f getPosition() const;
    sf::Vector2f getDirection() const;
    float getSpeed() const;
    ArrowHitMode getHitMode() const;

private:
    sf::Sprite sprite;
    sf::Vector2f startPosition;
    sf::Vector2f position;
    sf::Vector2f direction;
    float speed;
    float distanceTraveled;
    float maxDistance;
    ArrowHitMode hitMode;
    bool active;
    std::unordered_set<Enemy*> hitEnemies;
};

#endif // ARROW_PROJECTILE_HPP
