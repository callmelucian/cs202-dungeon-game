#include "collision-solver.hpp"
#include "../entities/character.hpp"
#include "math-utility.hpp"
#include <cmath>
#include <algorithm>
#include <limits>

bool CollisionSolver::aabbIntersect(const sf::FloatRect& rect1, const sf::FloatRect& rect2) {
    return Math::checkAABBIntersection(rect1, rect2);
}

bool CollisionSolver::circleIntersect(const sf::Vector2f& center1, float radius1, const sf::Vector2f& center2, float radius2) {
    float dx = center2.x - center1.x;
    float dy = center2.y - center1.y;
    float distanceSq = dx * dx + dy * dy;
    float radiusSum = radius1 + radius2;
    return distanceSq <= (radiusSum * radiusSum);
}

void CollisionSolver::resolveAABB(Character& character, const std::vector<sf::FloatRect>& obstacles, float deltaTime) {
    sf::Vector2f vel = character.getVelocity();
    sf::Vector2f pos = character.getPosition();

    // Apply full movement
    pos += vel * deltaTime;
    character.setPosition(pos);

    // Resolve each overlapping obstacle using minimum penetration depth
    for (const auto& obs : obstacles) {
        sf::FloatRect bounds = character.getBounds();

        // Strict < check: touching (flush) rects are NOT overlapping
        bool overlapping =
            bounds.position.x < obs.position.x + obs.size.x &&
            bounds.position.x + bounds.size.x > obs.position.x &&
            bounds.position.y < obs.position.y + obs.size.y &&
            bounds.position.y + bounds.size.y > obs.position.y;

        if (!overlapping) continue;

        // Calculate penetration depth on all four sides
        float pushLeft  = (bounds.position.x + bounds.size.x) - obs.position.x;       // push player left
        float pushRight = (obs.position.x + obs.size.x) - bounds.position.x;           // push player right
        float pushUp    = (bounds.position.y + bounds.size.y) - obs.position.y;         // push player up
        float pushDown  = (obs.position.y + obs.size.y) - bounds.position.y;            // push player down

        // Find the minimum push distance
        float minPush = pushLeft;
        int axis = 0; // 0=left, 1=right, 2=up, 3=down

        if (pushRight < minPush) { minPush = pushRight; axis = 1; }
        if (pushUp    < minPush) { minPush = pushUp;    axis = 2; }
        if (pushDown  < minPush) { minPush = pushDown;  axis = 3; }

        pos = character.getPosition();
        vel = character.getVelocity();

        switch (axis) {
            case 0: pos.x -= minPush; vel.x = std::min(vel.x, 0.f); break; // push left
            case 1: pos.x += minPush; vel.x = std::max(vel.x, 0.f); break; // push right
            case 2: pos.y -= minPush; vel.y = std::min(vel.y, 0.f); break; // push up
            case 3: pos.y += minPush; vel.y = std::max(vel.y, 0.f); break; // push down
        }

        character.setPosition(pos);
        character.setVelocity(vel);
    }
}


// Helper to check intersection of two line segments: AB and CD
static bool lineSegmentIntersection(const sf::Vector2f& A, const sf::Vector2f& B,
                                    const sf::Vector2f& C, const sf::Vector2f& D,
                                    sf::Vector2f& outIntersection) {
    sf::Vector2f r = B - A;
    sf::Vector2f s = D - C;
    float rxs = r.x * s.y - r.y * s.x;

    if (std::abs(rxs) < 1e-6f) {
        return false; // Parallel or collinear
    }

    sf::Vector2f CminusA = C - A;
    float t = (CminusA.x * s.y - CminusA.y * s.x) / rxs;
    float u = (CminusA.x * r.y - CminusA.y * r.x) / rxs;

    if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
        outIntersection = A + t * r;
        return true;
    }
    return false;
}

bool CollisionSolver::lineIntersectsRect(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::FloatRect& rect, sf::Vector2f& intersectionPoint) {
    // If the starting point is inside the rect, the intersection is immediate
    if (rect.contains(p1)) {
        intersectionPoint = p1;
        return true;
    }

    // Check intersection with all 4 edges of the rectangle
    sf::Vector2f corners[4] = {
        rect.position,                                     // Top-Left
        {rect.position.x + rect.size.x, rect.position.y},  // Top-Right
        rect.position + rect.size,                         // Bottom-Right
        {rect.position.x, rect.position.y + rect.size.y}   // Bottom-Left
    };

    bool hit = false;
    float closestDistSq = std::numeric_limits<float>::max();
    sf::Vector2f bestIntersection;

    for (int i = 0; i < 4; ++i) {
        sf::Vector2f edgeStart = corners[i];
        sf::Vector2f edgeEnd = corners[(i + 1) % 4];
        sf::Vector2f inter;
        if (lineSegmentIntersection(p1, p2, edgeStart, edgeEnd, inter)) {
            float distSq = Math::distanceSq(p1, inter);
            if (distSq < closestDistSq) {
                closestDistSq = distSq;
                bestIntersection = inter;
                hit = true;
            }
        }
    }

    if (hit) {
        intersectionPoint = bestIntersection;
    }
    return hit;
}

bool CollisionSolver::lineIntersectsCircle(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& center, float radius, sf::Vector2f& intersectionPoint) {
    sf::Vector2f d = p2 - p1;
    float lenSq = d.x * d.x + d.y * d.y;
    if (lenSq < 1e-6f) {
        // Line is a point
        if (Math::distanceSq(p1, center) <= radius * radius) {
            intersectionPoint = p1;
            return true;
        }
        return false;
    }

    // Project circle center onto the segment
    float t = Math::dot(center - p1, d) / lenSq;
    t = std::clamp(t, 0.0f, 1.0f);
    sf::Vector2f closestPoint = p1 + t * d;

    // Check if the closest point is within the radius
    float distSq = Math::distanceSq(closestPoint, center);
    if (distSq <= radius * radius) {
        sf::Vector2f f = p1 - center;
        float a = lenSq;
        float b = 2.0f * Math::dot(f, d);
        float c = Math::dot(f, f) - radius * radius;

        float discriminant = b * b - 4.0f * a * c;
        if (discriminant >= 0.0f) {
            discriminant = std::sqrt(discriminant);
            float t1 = (-b - discriminant) / (2.0f * a);
            float t2 = (-b + discriminant) / (2.0f * a);

            if (t1 >= 0.0f && t1 <= 1.0f) {
                intersectionPoint = p1 + t1 * d;
                return true;
            }
            if (t2 >= 0.0f && t2 <= 1.0f) {
                intersectionPoint = p1 + t2 * d;
                return true;
            }
        }
        intersectionPoint = closestPoint;
        return true;
    }

    return false;
}
