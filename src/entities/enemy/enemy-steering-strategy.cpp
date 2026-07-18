#include "enemy-steering-strategy.hpp"
#include "enemy.hpp"
#include "../player.hpp"
#include "../../chambers/chamber.hpp"
#include <cmath>

static sf::Vector2f normalize(const sf::Vector2f& source) {
    float length = std::sqrt(source.x * source.x + source.y * source.y);
    if (length != 0)
        return sf::Vector2f(source.x / length, source.y / length);
    else
        return source;
}

sf::Vector2f SeekStrategy::calculateSteering(Enemy& enemy, const Player& player, const Chamber& chamber) {
    // Seek: Use BFS pathfinding to find the next waypoint
    std::vector<sf::Vector2f> path = chamber.findPath(enemy.getPosition(), player.getPosition());
    if (!path.empty()) {
        sf::Vector2f targetWaypoint = path.front();
        sf::Vector2f direction = targetWaypoint - enemy.getPosition();
        return normalize(direction);
    }
    
    // Fallback if no path is found
    sf::Vector2f direction = player.getPosition() - enemy.getPosition();
    return normalize(direction);
}
sf::Vector2f EvadeStrategy::calculateSteering(Enemy& enemy, const Player& player, const Chamber& chamber) {
    // Evade: Desired direction is from the Player towards the Enemy (moving away)
    sf::Vector2f direction = enemy.getPosition() - player.getPosition();
    return normalize(direction);
}