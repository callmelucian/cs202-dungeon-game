#include "projectile.hpp"
#include "../../utils/math-utility.hpp"

Projectile::Projectile(sf::Vector2f startPos,
                       sf::Vector2f direction,
                       float speed,
                       float maxDistance,
                       float lifetime)
    : startPosition(startPos),
      position(startPos),
      direction(Math::normalize(direction)),
      velocity(this->direction * speed),
      speed(speed),
      distanceTraveled(0.0f),
      maxDistance(maxDistance),
      lifetime(lifetime),
      active(true) {}

void Projectile::updateMotion(float dt) {
    if (!active) return;

    float step = speed * dt;
    position += velocity * dt;
    distanceTraveled += step;
    lifetime -= dt;

    if (lifetime <= 0.0f || distanceTraveled >= maxDistance) {
        active = false;
    }
}
