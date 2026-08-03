#ifndef PATHFINDER_HPP
#define PATHFINDER_HPP

#include <vector>
#include <SFML/Graphics.hpp>

class Chamber;

class Pathfinder {
public:
    static bool isWalkable(sf::Vector2f position, const std::vector<std::vector<std::string>>& grid);
    static std::vector<sf::Vector2f> findPath(sf::Vector2f start, sf::Vector2f target, const std::vector<std::vector<std::string>>& grid);
    static std::vector<sf::Vector2f> findPath(sf::Vector2f start, sf::Vector2f target, const Chamber& chamber);
};

#endif // PATHFINDER_HPP
