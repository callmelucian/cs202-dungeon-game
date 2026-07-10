#ifndef ENEMY_STEERING_STRATEGY_HPP
#define ENEMY_STEERING_STRATEGY_HPP

#include <SFML/System/Vector2.hpp>

// Forward declarations to avoid cyclic includes
class Enemy;
class Player;
class Chamber;

class EnemySteeringStrategy {
public:
    virtual ~EnemySteeringStrategy() = default;

    // Returns the desired velocity vector for the enemy
    virtual sf::Vector2f calculateSteering(Enemy& enemy, const Player& player, const Chamber& chamber) = 0;
};

// Concrete Strategy: Seeks directly towards the player
class SeekStrategy : public EnemySteeringStrategy {
public:
    sf::Vector2f calculateSteering(Enemy& enemy, const Player& player, const Chamber& chamber) override;
};

// Concrete Strategy: Evades by moving directly away from the player
class EvadeStrategy : public EnemySteeringStrategy {
public:
    sf::Vector2f calculateSteering(Enemy& enemy, const Player& player, const Chamber& chamber) override;
};

#endif // ENEMY_STEERING_STRATEGY_HPP