#include "camera.hpp"
#include "math-utility.hpp"
#include <cmath>
#include <algorithm>

Camera::Camera()
    : windowSize(1280.0f, 720.0f),
      currentCenter(0.0f, 0.0f),
      targetCenter(0.0f, 0.0f),
      currentZoom(0.5f),
      targetZoom(0.5f),
      minZoom(0.5f),
      positionSpeed(8.0f),
      zoomSpeed(10.0f),
      zoomStep(0.05f) {
    view.setSize({windowSize.x * currentZoom, windowSize.y * currentZoom});
    view.setCenter(currentCenter);
}

Camera::Camera(sf::Vector2f winSize, float initialZoom)
    : windowSize(winSize),
      currentCenter(0.0f, 0.0f),
      targetCenter(0.0f, 0.0f),
      currentZoom(initialZoom),
      targetZoom(initialZoom),
      minZoom(0.5f),
      positionSpeed(8.0f),
      zoomSpeed(10.0f),
      zoomStep(0.05f) {
    view.setSize({windowSize.x * currentZoom, windowSize.y * currentZoom});
    view.setCenter(currentCenter);
}

void Camera::init(sf::Vector2f winSize, float initialZoom) {
    windowSize = winSize;
    currentZoom = initialZoom;
    targetZoom = initialZoom;
    view.setSize({windowSize.x * currentZoom, windowSize.y * currentZoom});
    view.setCenter(currentCenter);
}

void Camera::setTargetCenter(const sf::Vector2f& target) {
    targetCenter = target;
}

void Camera::zoomBy(float delta, float maxAllowedZoomOut) {
    targetZoom -= delta * zoomStep;
    targetZoom = std::clamp(targetZoom, minZoom, maxAllowedZoomOut);
}

void Camera::setTargetZoom(float zoom, float maxAllowedZoomOut) {
    float effectiveMax = std::max(minZoom, maxAllowedZoomOut);
    targetZoom = std::clamp(zoom, minZoom, effectiveMax);
}

void Camera::snapToTarget() {
    currentCenter = targetCenter;
    currentZoom = targetZoom;
    view.setSize({windowSize.x * currentZoom, windowSize.y * currentZoom});
    view.setCenter(currentCenter);
}

void Camera::update(float deltaTime, const sf::FloatRect& mapBounds) {
    if (deltaTime <= 0.0f) return;

    // 1. Frame-rate independent exponential lerp for zoom
    float tZoom = 1.0f - std::exp(-zoomSpeed * deltaTime);
    currentZoom = Math::lerp(currentZoom, targetZoom, tZoom);

    // Update view size based on interpolated zoom
    float viewWidth = windowSize.x * currentZoom;
    float viewHeight = windowSize.y * currentZoom;
    view.setSize({viewWidth, viewHeight});

    // 2. Map boundary calculations & target alignment
    float halfW = viewWidth / 2.0f;
    float halfH = viewHeight / 2.0f;

    float minX = mapBounds.position.x;
    float minY = mapBounds.position.y;
    float maxX = minX + mapBounds.size.x;
    float maxY = minY + mapBounds.size.y;

    // If map width fits inside view width, center target horizontally on map center
    if (mapBounds.size.x <= viewWidth) {
        targetCenter.x = minX + mapBounds.size.x / 2.0f;
    }
    // If map height fits inside view height, center target vertically on map center
    if (mapBounds.size.y <= viewHeight) {
        targetCenter.y = minY + mapBounds.size.y / 2.0f;
    }

    // 3. Frame-rate independent exponential lerp for position
    float tPos = 1.0f - std::exp(-positionSpeed * deltaTime);
    currentCenter = Math::lerp(currentCenter, targetCenter, tPos);

    // 4. Strict boundary clamping and central alignment
    float camX = currentCenter.x;
    float camY = currentCenter.y;

    if (mapBounds.size.x <= viewWidth) {
        camX = minX + mapBounds.size.x / 2.0f;
    } else {
        if (camX - halfW < minX) camX = minX + halfW;
        if (camX + halfW > maxX) camX = maxX - halfW;
    }

    if (mapBounds.size.y <= viewHeight) {
        camY = minY + mapBounds.size.y / 2.0f;
    } else {
        if (camY - halfH < minY) camY = minY + halfH;
        if (camY + halfH > maxY) camY = maxY - halfH;
    }

    currentCenter = sf::Vector2f(camX, camY);
    view.setCenter(currentCenter);
}

const sf::View& Camera::getView() const {
    return view;
}

sf::Vector2f Camera::getCenter() const {
    return currentCenter;
}

sf::Vector2f Camera::getTargetCenter() const {
    return targetCenter;
}

float Camera::getCurrentZoom() const {
    return currentZoom;
}

float Camera::getTargetZoom() const {
    return targetZoom;
}

void Camera::setPositionSpeed(float speed) {
    positionSpeed = speed;
}

void Camera::setZoomSpeed(float speed) {
    zoomSpeed = speed;
}

void Camera::setMinZoom(float minZ) {
    minZoom = minZ;
}
