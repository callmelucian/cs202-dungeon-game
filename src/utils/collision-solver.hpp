#ifndef COLLISION_SOLVER_HPP
#define COLLISION_SOLVER_HPP

/**
 * @file collision-solver.hpp
 * @brief Stateless static helper class for AABB and circle intersections and collision resolution.
 */

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

class Character;

#include <variant>

// Hitbox definitions
struct CircleHitbox {
    sf::Vector2f center;
    float radius;
};

struct ConeHitbox {
    sf::Vector2f origin;
    sf::Vector2f direction; // Normalized direction vector
    float length;
    float angleDegrees;     // Total arc angle in degrees
};

struct LineHitbox {
    sf::Vector2f start;
    sf::Vector2f end;
};

using Hitbox = std::variant<CircleHitbox, ConeHitbox, LineHitbox>;

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

    // Hitbox collisions
    static bool checkCollision(const CircleHitbox& circle, const sf::FloatRect& rect);
    static bool checkCollision(const ConeHitbox& cone, const sf::FloatRect& rect);
    static bool checkCollision(const LineHitbox& line, const sf::FloatRect& rect);
    static bool checkCollision(const Hitbox& hitbox, const sf::FloatRect& rect);

    // Helpers for debug rendering
    static void drawDebug(sf::RenderTarget& target, const Hitbox& hitbox, sf::Color color = sf::Color(255, 0, 0, 100));
};

#endif // COLLISION_SOLVER_HPP
