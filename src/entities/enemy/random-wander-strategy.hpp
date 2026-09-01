#ifndef RANDOM_WANDER_STRATEGY_HPP
#define RANDOM_WANDER_STRATEGY_HPP

#include "enemy-steering-strategy.hpp"
#include <SFML/System/Vector2.hpp>

// RandomWanderStrategy: Autonomous steering strategy that picks periodic random directions
// within a local wandering radius. Ideal for tutorial training dummies and isolated platform enemies.
class RandomWanderStrategy : public EnemySteeringStrategy {
public:
    explicit RandomWanderStrategy(float changeInterval = 1.8f, float wanderRadius = 120.0f);
    virtual ~RandomWanderStrategy() override = default;

    sf::Vector2f calculateSteering(Enemy& enemy, const Player& player, const Chamber& chamber) override;
    void onWallHit() override;

private:
    float changeInterval;
    float timer;
    float wanderRadius;
    sf::Vector2f currentDir;
    sf::Vector2f originPos;
    bool hasOrigin;

    void pickNewDirection(const sf::Vector2f& currentPos);
};

#endif // RANDOM_WANDER_STRATEGY_HPP
