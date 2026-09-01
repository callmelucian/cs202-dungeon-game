#include "random-wander-strategy.hpp"
#include "enemy.hpp"
#include "../../utils/math-utility.hpp"
#include <cstdlib>
#include <cmath>

RandomWanderStrategy::RandomWanderStrategy(float changeInterval, float wanderRadius)
    : changeInterval(changeInterval),
      timer(0.0f),
      wanderRadius(wanderRadius),
      currentDir(0.0f, 0.0f),
      originPos(0.0f, 0.0f),
      hasOrigin(false)
{
}

void RandomWanderStrategy::pickNewDirection(const sf::Vector2f& currentPos) {
    if (!hasOrigin) {
        originPos = currentPos;
        hasOrigin = true;
    }

    // Check if enemy moved outside wander radius
    sf::Vector2f diff = currentPos - originPos;
    float dist = Math::length(diff);
    if (dist > wanderRadius && dist > 0.001f) {
        // Steer back towards origin
        currentDir = Math::normalize(-diff);
        timer = changeInterval;
        return;
    }

    // Pick a random angle in [0, 2*pi]
    float angle = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 6.2831853f;
    currentDir = sf::Vector2f(std::cos(angle), std::sin(angle));
    timer = changeInterval;
}

sf::Vector2f RandomWanderStrategy::calculateSteering(Enemy& enemy, const Player& /*player*/, const Chamber& /*chamber*/) {
    if (!hasOrigin) {
        originPos = enemy.getPosition();
        hasOrigin = true;
        pickNewDirection(enemy.getPosition());
    }

    timer -= 0.016f; // approximate delta tick for steering strategy
    if (timer <= 0.0f) {
        pickNewDirection(enemy.getPosition());
    }

    // Check bounds
    sf::Vector2f diff = enemy.getPosition() - originPos;
    float dist = Math::length(diff);
    if (dist > wanderRadius && dist > 0.001f) {
        currentDir = Math::normalize(-diff);
    }

    return currentDir;
}

void RandomWanderStrategy::onWallHit() {
    // Reverse and randomize direction on collision
    currentDir = -currentDir;
    timer = changeInterval * 0.5f;
}
