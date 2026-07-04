#ifndef MATH_UTILITY_HPP
#define MATH_UTILITY_HPP

/**
 * @file math-utility.hpp
 * @brief Standard mathematical and 2D vector utility functions for game mechanics.
 * 
 * Provides vector magnitude, normalization, dot/cross products, distance checks,
 * interpolation, projection, reflection, and angle conversions.
 */

#include <SFML/System/Vector2.hpp>

namespace Math {

    // Common mathematical constants
    constexpr float PI = 3.14159265f;
    constexpr float DEG_TO_RAD = PI / 180.0f;
    constexpr float RAD_TO_DEG = 180.0f / PI;

    /**
     * @brief Computes the squared length (magnitude) of a 2D vector.
     * Fast option to avoid std::sqrt.
     */
    float lengthSq(const sf::Vector2f& v);

    /**
     * @brief Computes the length (magnitude) of a 2D vector.
     */
    float length(const sf::Vector2f& v);

    /**
     * @brief Returns a normalized (unit length) copy of a vector.
     * Safe against zero-length or extremely small vectors (returns zero vector in those cases).
     */
    sf::Vector2f normalize(const sf::Vector2f& v);

    /**
     * @brief Computes the dot product of two vectors.
     */
    float dot(const sf::Vector2f& v1, const sf::Vector2f& v2);

    /**
     * @brief Computes the 2D cross product magnitude of two vectors (v1.x * v2.y - v1.y * v2.x).
     */
    float cross(const sf::Vector2f& v1, const sf::Vector2f& v2);

    /**
     * @brief Computes the squared distance between two points.
     * Fast option to avoid std::sqrt.
     */
    float distanceSq(const sf::Vector2f& p1, const sf::Vector2f& p2);

    /**
     * @brief Computes the Euclidean distance between two points.
     */
    float distance(const sf::Vector2f& p1, const sf::Vector2f& p2);

    /**
     * @brief Checks if the distance between two points is strictly less than a threshold.
     * Performs a fast squared distance check.
     */
    bool isDistanceLessThan(const sf::Vector2f& p1, const sf::Vector2f& p2, float threshold);

    /**
     * @brief Linearly interpolates between two floats.
     */
    float lerp(float a, float b, float t);

    /**
     * @brief Linearly interpolates between two vectors.
     */
    sf::Vector2f lerp(const sf::Vector2f& v1, const sf::Vector2f& v2, float t);

    /**
     * @brief Projects vector v onto a normal vector.
     */
    sf::Vector2f project(const sf::Vector2f& v, const sf::Vector2f& normal);

    /**
     * @brief Reflects a velocity vector v off a normal vector.
     */
    sf::Vector2f reflect(const sf::Vector2f& v, const sf::Vector2f& normal);

    /**
     * @brief Converts radians to degrees.
     */
    float radToDeg(float radians);

    /**
     * @brief Converts degrees to radians.
     */
    float degToRad(float degrees);

    /**
     * @brief Computes the angle of a 2D vector in radians (relative to standard coordinate system).
     * Returns an angle in the range [-PI, PI].
     */
    float angle(const sf::Vector2f& v);

    /**
     * @brief Generates a unit vector representing an angle in radians.
     */
    sf::Vector2f vectorFromAngle(float angleInRadians);

    /**
     * @brief Clamps a vector's magnitude to a maximum length.
     */
    sf::Vector2f clampLength(const sf::Vector2f& v, float maxLength);

} // namespace Math

#endif // MATH_UTILITY_HPP
