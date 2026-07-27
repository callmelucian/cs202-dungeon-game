#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

/**
 * @file camera.hpp
 * @brief Handles 2D game camera positioning, linear interpolation (lerp),
 *        smooth zoom transitions, and map boundary constraints.
 */
class Camera {
public:
    Camera();
    Camera(sf::Vector2f windowSize, float initialZoom = 0.5f);

    /**
     * @brief Sets up window size and initial zoom level.
     */
    void init(sf::Vector2f windowSize, float initialZoom = 0.5f);

    /**
     * @brief Set target position for the camera center.
     */
    void setTargetCenter(const sf::Vector2f& target);

    /**
     * @brief Adjust target zoom by delta (e.g. from mouse wheel scroll).
     */
    void zoomBy(float delta, float maxAllowedZoomOut);

    /**
     * @brief Directly sets the target zoom value, clamped to [minZoom, maxAllowedZoomOut].
     */
    void setTargetZoom(float zoom, float maxAllowedZoomOut);

    /**
     * @brief Instantly moves camera to current target position and zoom level.
     */
    void snapToTarget();

    /**
     * @brief Updates smooth position and zoom lerp, applying boundary clamping.
     * @param deltaTime Elapsed time in seconds.
     * @param mapBounds Rect defining valid camera view limits in world coordinates.
     */
    void update(float deltaTime, const sf::FloatRect& mapBounds);

    /**
     * @brief Access the SFML View to set on RenderWindow.
     */
    const sf::View& getView() const;

    sf::Vector2f getCenter() const;
    sf::Vector2f getTargetCenter() const;
    float getCurrentZoom() const;
    float getTargetZoom() const;

    void setPositionSpeed(float speed);
    void setZoomSpeed(float speed);
    void setMinZoom(float minZ);

private:
    sf::View view;
    sf::Vector2f windowSize;

    sf::Vector2f currentCenter;
    sf::Vector2f targetCenter;

    float currentZoom;
    float targetZoom;

    float minZoom;
    float positionSpeed; // Factor for lerp exponential decay
    float zoomSpeed;     // Factor for zoom lerp decay
    float zoomStep;      // Zoom increment per scroll unit
};

#endif // CAMERA_HPP
