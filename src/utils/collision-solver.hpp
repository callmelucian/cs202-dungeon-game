#ifndef COLLISION_SOLVER_HPP
#define COLLISION_SOLVER_HPP

/**
 * @file collision-solver.hpp
 * @brief Stateless static helper class for AABB and circle intersections and collision resolution.
 */

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

class Character;

class CollisionSolver {
public:
    // AABB intersection check
    static bool aabbIntersect(const sf::FloatRect& rect1, const sf::FloatRect& rect2);

    // Circle intersection check
    static bool circleIntersect(const sf::Vector2f& center1, float radius1, const sf::Vector2f& center2, float radius2);

    // Resolves AABB collision between a character and solid walls.
    // Integrates velocity per-axis then snaps the character flush to any wall it hits.
    static void resolveAABB(Character& character, const std::vector<sf::FloatRect>& obstacles, float deltaTime);

    // Checks if a line segment intersects a rectangle and returns the closest intersection point
    static bool lineIntersectsRect(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::FloatRect& rect, sf::Vector2f& intersectionPoint);

    // Checks if a line segment intersects a circle and returns the closest intersection point
    static bool lineIntersectsCircle(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& center, float radius, sf::Vector2f& intersectionPoint);
};

#endif // COLLISION_SOLVER_HPP
