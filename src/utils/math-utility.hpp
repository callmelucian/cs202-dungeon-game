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
#include <SFML/Graphics/Rect.hpp>
#include <vector>

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

    /**
     * @brief Selects an index from a vector of weights based on their relative probabilities.
     * @param weights A list of numerical weights.
     * @return The selected index. If weights are empty, returns 0.
     */
    int weightedRandom(const std::vector<float>& weights);

    /**
     * @brief Decides an outcome based on a probability in the range [0.0, 1.0].
     * @param probability The probability of returning true (0.0 to 1.0).
     * @return True if the roll succeeds, false otherwise.
     */
    bool decide(float probability);

    /**
     * @brief Checks if two Axis-Aligned Bounding Boxes (AABB) intersect using float rects.
     */
    bool checkAABBIntersection(const sf::FloatRect& rect1, const sf::FloatRect& rect2);

    /**
     * @brief Checks if two Axis-Aligned Bounding Boxes (AABB) intersect using min/max bounds.
     */
    bool checkAABBIntersection(
        float minX1, float minY1, float maxX1, float maxY1,
        float minX2, float minY2, float maxX2, float maxY2
    );

} // namespace Math

#endif // MATH_UTILITY_HPP
