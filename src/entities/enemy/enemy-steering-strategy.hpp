#ifndef ENEMY_STEERING_STRATEGY_HPP
#define ENEMY_STEERING_STRATEGY_HPP

#include <SFML/System/Vector2.hpp>
#include <vector>

// Forward declarations to avoid cyclic includes
class Enemy;
class Player;
class Chamber;

class EnemySteeringStrategy {
public:
    virtual ~EnemySteeringStrategy() = default;

    // Returns the desired normalised velocity direction for the enemy.
    virtual sf::Vector2f calculateSteering(Enemy& enemy, const Player& player, const Chamber& chamber) = 0;

    // Called whenever the enemy physically collides with a wall.
    // Implementations may use this to trigger a path replan.
    virtual void onWallHit() {}
};

// Concrete Strategy: Seeks towards the player using cached BFS pathfinding with
// waypoint-following.  The path is replanned only when:
//   (a) the cached path is exhausted,
//   (b) the player has moved more than REPLAN_DISTANCE_THRESHOLD cells since the
//       last plan was computed, or
//   (c) onWallHit() is called (physical wall collision detected by the solver).
//
// This avoids the per-frame BFS cost and fixes the "enemy stuck at wall" bug
// caused by the collision solver zeroing the velocity every frame while the
// stale path keeps pointing through the wall.
class SeekStrategy : public EnemySteeringStrategy {
private:
    std::vector<sf::Vector2f> cachedPath;
    bool needsReplan = true;

    // World-space position that was used as the target of the last planned path.
    sf::Vector2f lastTargetPos = {-1e9f, -1e9f};

    // How far (in cells) the player must move before we force a replan even if
    // the cached path is not yet exhausted.
    static constexpr float REPLAN_DISTANCE_THRESHOLD = 1.5f;

    // An enemy is considered to have "arrived" at a waypoint when it is within
    // this fraction of a cell from the waypoint centre.
    static constexpr float ARRIVE_RADIUS_FACTOR = 0.5f;

public:
    sf::Vector2f calculateSteering(Enemy& enemy, const Player& player, const Chamber& chamber) override;

    // Marks the cached path as stale so it will be replanned on the next
    // calculateSteering call.
    void onWallHit() override;
};

// Concrete Strategy: Evades by moving directly away from the player
class EvadeStrategy : public EnemySteeringStrategy {
public:
    sf::Vector2f calculateSteering(Enemy& enemy, const Player& player, const Chamber& chamber) override;
};

#endif // ENEMY_STEERING_STRATEGY_HPP