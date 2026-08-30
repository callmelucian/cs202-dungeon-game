#ifndef PROJECTILE_HPP
#define PROJECTILE_HPP

#include <SFML/Graphics.hpp>

/**
 * @brief Abstract base class for all physical and magical in-flight projectiles in the game.
 * Encapsulates position kinematics, flight trajectory, velocity, travel distance, lifetime,
 * and active state tracking.
 */
class Projectile {
protected:
    sf::Vector2f startPosition;
    sf::Vector2f position;
    sf::Vector2f direction;
    sf::Vector2f velocity;
    float speed;
    float distanceTraveled;
    float maxDistance;
    float lifetime;
    bool active;

public:
    Projectile(sf::Vector2f startPos,
               sf::Vector2f direction,
               float speed,
               float maxDistance = 10000.0f,
               float lifetime = 10.0f);
    virtual ~Projectile() = default;

    virtual void updateMotion(float dt);
    virtual void draw(sf::RenderTarget& target) const = 0;

    bool isActive() const { return active; }
    void deactivate() { active = false; }

    sf::Vector2f getStartPosition() const { return startPosition; }
    sf::Vector2f getPosition() const { return position; }
    sf::Vector2f getDirection() const { return direction; }
    sf::Vector2f getVelocity() const { return velocity; }
    float getSpeed() const { return speed; }
    float getDistanceTraveled() const { return distanceTraveled; }
    float getMaxDistance() const { return maxDistance; }
    float getLifetime() const { return lifetime; }
};

#endif // PROJECTILE_HPP
