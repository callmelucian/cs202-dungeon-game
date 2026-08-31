#include "enemy-steering-strategy.hpp"
#include "enemy.hpp"
#include "../player.hpp"
#include "../../chambers/chamber.hpp"
#include "../../utils/pathfinder.hpp"
#include "../../utils/math-utility.hpp"
#include "../../global-settings/setting-manager.hpp"
#include <cmath>

// ---------------------------------------------------------------------------
// SeekStrategy
// ---------------------------------------------------------------------------

sf::Vector2f SeekStrategy::calculateSteering(Enemy& enemy, const Player& player, const Chamber& chamber) {
    float cellSize = SettingManager::getInstance().getCellSize();

    // -----------------------------------------------------------------------
    // Replan decision
    // -----------------------------------------------------------------------
    // Force a replan if the player has drifted more than REPLAN_DISTANCE_THRESHOLD
    // cells since the path was last computed.  This keeps the path fresh even
    // when the enemy is following a long route while the player moves away.
    if (!needsReplan) {
        float playerDrift = Math::distance(player.getPosition(), lastTargetPos);
        if (playerDrift > cellSize * REPLAN_DISTANCE_THRESHOLD) {
            needsReplan = true;
        }
    }

    // Recompute via BFS whenever flagged (wall hit, path exhausted, first call,
    // or player drifted).
    if (needsReplan || cachedPath.empty()) {
        cachedPath = Pathfinder::findPath(enemy.getPosition(), player.getPosition(), chamber);
        lastTargetPos = player.getPosition();
        needsReplan = false;
    }

    // -----------------------------------------------------------------------
    // Waypoint advancement
    // -----------------------------------------------------------------------
    // Pop waypoints the enemy has already reached.  Using a threshold of half a
    // cell size prevents the steering direction from flipping when the enemy's
    // float position straddles a cell-centre boundary (Bug 4).
    float arrivalRadius = cellSize * ARRIVE_RADIUS_FACTOR;
    while (!cachedPath.empty() &&
           Math::distance(enemy.getPosition(), cachedPath.front()) < arrivalRadius) {
        cachedPath.erase(cachedPath.begin());
    }

    // -----------------------------------------------------------------------
    // Direction output
    // -----------------------------------------------------------------------
    if (!cachedPath.empty()) {
        sf::Vector2f toWaypoint = cachedPath.front() - enemy.getPosition();
        float len = Math::length(toWaypoint);
        if (len > 0.0001f) {
            return toWaypoint / len;
        }
    }

    // Fallback: direct line to player when path is empty (open line-of-sight or
    // pathfinder returned nothing).
    sf::Vector2f toPlayer = player.getPosition() - enemy.getPosition();
    float len = Math::length(toPlayer);
    if (len > 0.0001f) {
        return toPlayer / len;
    }
    return {0.f, 0.f};
}

void SeekStrategy::onWallHit() {
    // Mark the cached path stale so calculateSteering replans on the very next
    // frame, finding a route that goes around the obstacle rather than pressing
    // into it (fixes Bug 2: velocity zeroed by collision solver → stale path
    // repeats the collision indefinitely).
    needsReplan = true;
}

// ---------------------------------------------------------------------------
// EvadeStrategy
// ---------------------------------------------------------------------------

sf::Vector2f EvadeStrategy::calculateSteering(Enemy& enemy, const Player& player, const Chamber& chamber) {
    float cellSize = SettingManager::getInstance().getCellSize();
    sf::Vector2f exitPos = chamber.getExitPosition();

    // If a valid exit position is set on the chamber (e.g. PreventChamber carrier escape),
    // carrier navigates toward the exit using pathfinding!
    if (exitPos.x >= 0.0f) {
        if (needsReplan || cachedPath.empty()) {
            cachedPath = Pathfinder::findPath(enemy.getPosition(), exitPos, chamber);
            lastTargetPos = exitPos;
            needsReplan = false;
        }

        float arrivalRadius = cellSize * ARRIVE_RADIUS_FACTOR;
        while (!cachedPath.empty() &&
               Math::distance(enemy.getPosition(), cachedPath.front()) < arrivalRadius) {
            cachedPath.erase(cachedPath.begin());
        }

        if (!cachedPath.empty()) {
            sf::Vector2f toWaypoint = cachedPath.front() - enemy.getPosition();
            float len = Math::length(toWaypoint);
            if (len > 0.0001f) {
                return toWaypoint / len;
            }
        }

        sf::Vector2f toExit = exitPos - enemy.getPosition();
        float len = Math::length(toExit);
        if (len > 0.0001f) {
            return toExit / len;
        }
    }

    // Fallback: Evade from player
    sf::Vector2f toEnemy = enemy.getPosition() - player.getPosition();
    float len = Math::length(toEnemy);
    if (len > 0.0001f) {
        return toEnemy / len;
    }
    return {0.f, 0.f};
}

void EvadeStrategy::onWallHit() {
    needsReplan = true;
}