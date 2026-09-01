#include "enemy-steering-strategy.hpp"
#include "enemy.hpp"
#include "../player.hpp"
#include "../../chambers/chamber.hpp"
#include "../../utils/pathfinder.hpp"
#include "../../utils/math-utility.hpp"
#include "../../global-settings/setting-manager.hpp"
#include <cmath>

static sf::Vector2f followGridPath(const sf::Vector2f& enemyPos,
                                   std::vector<sf::Vector2f>& path,
                                   float cellSize) {
    if (path.empty()) return {0.0f, 0.0f};

    // Pop waypoints that the entity has reached.
    // Using a tight threshold ensures the entity fully enters the cell before turning.
    while (!path.empty()) {
        float dist = Math::distance(enemyPos, path.front());
        if (path.size() == 1) {
            if (dist < 4.0f) {
                path.erase(path.begin());
                return {0.0f, 0.0f};
            }
            break;
        }

        if (dist < cellSize * 0.35f) {
            path.erase(path.begin());
        } else {
            break;
        }
    }

    if (path.empty()) return {0.0f, 0.0f};

    sf::Vector2f target = path.front();
    float dx = target.x - enemyPos.x;
    float dy = target.y - enemyPos.y;

    // Axis-aligned steering with corridor centering:
    // If predominantly horizontal movement:
    //   Move along X (+/- 1.0), and gently center along Y to stay in corridor.
    // If predominantly vertical movement:
    //   Move along Y (+/- 1.0), and gently center along X to stay in corridor.
    sf::Vector2f steerDir(0.0f, 0.0f);
    if (std::abs(dx) > std::abs(dy)) {
        steerDir.x = (dx > 0.0f) ? 1.0f : -1.0f;
        steerDir.y = std::clamp(dy / (cellSize * 0.5f), -0.5f, 0.5f);
    } else {
        steerDir.x = std::clamp(dx / (cellSize * 0.5f), -0.5f, 0.5f);
        steerDir.y = (dy > 0.0f) ? 1.0f : -1.0f;
    }

    float len = Math::length(steerDir);
    if (len > 0.0001f) {
        return steerDir / len;
    }
    return {0.0f, 0.0f};
}

// ---------------------------------------------------------------------------
// SeekStrategy
// ---------------------------------------------------------------------------

sf::Vector2f SeekStrategy::calculateSteering(Enemy& enemy, const Player& player, const Chamber& chamber) {
    float cellSize = SettingManager::getInstance().getCellSize();

    // Replan if target player moved significantly
    if (!needsReplan) {
        float playerDrift = Math::distance(player.getPosition(), lastTargetPos);
        if (playerDrift > cellSize * REPLAN_DISTANCE_THRESHOLD) {
            needsReplan = true;
        }
    }

    if (needsReplan || cachedPath.empty()) {
        cachedPath = Pathfinder::findPath(enemy.getPosition(), player.getPosition(), chamber);
        lastTargetPos = player.getPosition();
        needsReplan = false;
    }

    if (!cachedPath.empty()) {
        sf::Vector2f dir = followGridPath(enemy.getPosition(), cachedPath, cellSize);
        if (Math::length(dir) > 0.0001f) {
            return dir;
        }
    }

    // Fallback: direct line to player when path is exhausted
    sf::Vector2f toPlayer = player.getPosition() - enemy.getPosition();
    float len = Math::length(toPlayer);
    if (len > 0.0001f) {
        return toPlayer / len;
    }
    return {0.f, 0.f};
}

void SeekStrategy::onWallHit() {
    if (cachedPath.empty()) {
        needsReplan = true;
    }
}

// ---------------------------------------------------------------------------
// EvadeStrategy
// ---------------------------------------------------------------------------

sf::Vector2f EvadeStrategy::calculateSteering(Enemy& enemy, const Player& player, const Chamber& chamber) {
    float cellSize = SettingManager::getInstance().getCellSize();
    sf::Vector2f exitPos = chamber.getExitPosition();

    // If exit position is set (e.g. PreventChamber carrier escape), navigate toward exit!
    if (exitPos.x >= 0.0f) {
        if (needsReplan || cachedPath.empty()) {
            cachedPath = Pathfinder::findPath(enemy.getPosition(), exitPos, chamber);
            lastTargetPos = exitPos;
            needsReplan = false;
        }

        if (!cachedPath.empty()) {
            sf::Vector2f dir = followGridPath(enemy.getPosition(), cachedPath, cellSize);
            if (Math::length(dir) > 0.0001f) {
                return dir;
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
    if (cachedPath.empty()) {
        needsReplan = true;
    }
}