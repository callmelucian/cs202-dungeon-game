#include "math-utility.hpp"
#include <cmath>
#include <random>

namespace Math {

    float lengthSq(const sf::Vector2f& v) {
        return v.x * v.x + v.y * v.y;
    }

    float length(const sf::Vector2f& v) {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }

    sf::Vector2f normalize(const sf::Vector2f& v) {
        float len = length(v);
        if (len > 0.00001f) {
            return sf::Vector2f(v.x / len, v.y / len);
        }
        return sf::Vector2f(0.0f, 0.0f);
    }

    float dot(const sf::Vector2f& v1, const sf::Vector2f& v2) {
        return v1.x * v2.x + v1.y * v2.y;
    }

    float cross(const sf::Vector2f& v1, const sf::Vector2f& v2) {
        return v1.x * v2.y - v1.y * v2.x;
    }

    float distanceSq(const sf::Vector2f& p1, const sf::Vector2f& p2) {
        float dx = p2.x - p1.x;
        float dy = p2.y - p1.y;
        return dx * dx + dy * dy;
    }

    float distance(const sf::Vector2f& p1, const sf::Vector2f& p2) {
        return std::sqrt(distanceSq(p1, p2));
    }

    bool isDistanceLessThan(const sf::Vector2f& p1, const sf::Vector2f& p2, float threshold) {
        return distanceSq(p1, p2) < (threshold * threshold);
    }

    float lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }

    sf::Vector2f lerp(const sf::Vector2f& v1, const sf::Vector2f& v2, float t) {
        return sf::Vector2f(lerp(v1.x, v2.x, t), lerp(v1.y, v2.y, t));
    }

    sf::Vector2f project(const sf::Vector2f& v, const sf::Vector2f& normal) {
        float normLenSq = lengthSq(normal);
        if (normLenSq > 0.00001f) {
            float dp = dot(v, normal);
            return normal * (dp / normLenSq);
        }
        return sf::Vector2f(0.0f, 0.0f);
    }

    sf::Vector2f reflect(const sf::Vector2f& v, const sf::Vector2f& normal) {
        sf::Vector2f normUnit = normalize(normal);
        return v - 2.0f * dot(v, normUnit) * normUnit;
    }

    float radToDeg(float radians) {
        return radians * RAD_TO_DEG;
    }

    float degToRad(float degrees) {
        return degrees * DEG_TO_RAD;
    }

    float angle(const sf::Vector2f& v) {
        return std::atan2(v.y, v.x);
    }

    sf::Vector2f vectorFromAngle(float angleInRadians) {
        return sf::Vector2f(std::cos(angleInRadians), std::sin(angleInRadians));
    }

    sf::Vector2f clampLength(const sf::Vector2f& v, float maxLength) {
        float lenSq = lengthSq(v);
        if (lenSq > maxLength * maxLength) {
            float len = std::sqrt(lenSq);
            return sf::Vector2f((v.x / len) * maxLength, (v.y / len) * maxLength);
        }
        return v;
    }

    int weightedRandom(const std::vector<float>& weights) {
        if (weights.empty()) {
            return 0;
        }
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::discrete_distribution<int> dist(weights.begin(), weights.end());
        return dist(gen);
    }

    bool decide(float probability) {
        if (probability <= 0.0f) return false;
        if (probability >= 1.0f) return true;

        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(gen) <= probability;
    }

    bool checkAABBIntersection(const sf::FloatRect& rect1, const sf::FloatRect& rect2) {
        return rect1.position.x <= rect2.position.x + rect2.size.x &&
               rect1.position.x + rect1.size.x >= rect2.position.x &&
               rect1.position.y <= rect2.position.y + rect2.size.y &&
               rect1.position.y + rect1.size.y >= rect2.position.y;
    }

    bool checkAABBIntersection(
        float minX1, float minY1, float maxX1, float maxY1,
        float minX2, float minY2, float maxX2, float maxY2
    ) {
        return minX1 <= maxX2 && maxX1 >= minX2 &&
               minY1 <= maxY2 && maxY1 >= minY2;
    }

} // namespace Math
