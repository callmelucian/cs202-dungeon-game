#include "exit-gate.hpp"
#include "../entities/player.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../ui/graphics/particle-system.hpp"
#include "../ui/graphics/aura-renderer.hpp"
#include <cmath>
#include <algorithm>

ExitGate::ExitGate(sf::Vector2f pos, bool active)
    : position(pos), active(active), pulseTimer(0.0f), particleTimer(0.0f) {
    
    float cellSize = SettingManager::getInstance().getCellSize();
    
    baseShape.setSize(sf::Vector2f(cellSize, cellSize));
    baseShape.setOrigin(sf::Vector2f(cellSize / 2.0f, cellSize / 2.0f));
    baseShape.setFillColor(sf::Color(80, 240, 160, 90));
    baseShape.setOutlineColor(sf::Color(120, 255, 200, 220));
    baseShape.setOutlineThickness(3.0f);
    baseShape.setPosition(position);

    innerShape.setSize(sf::Vector2f(cellSize * 0.6f, cellSize * 0.6f));
    innerShape.setOrigin(sf::Vector2f(cellSize * 0.3f, cellSize * 0.3f));
    innerShape.setFillColor(sf::Color(160, 255, 220, 140));
    innerShape.setPosition(position);
}

void ExitGate::setPosition(sf::Vector2f pos) {
    position = pos;
    baseShape.setPosition(position);
    innerShape.setPosition(position);
}

void ExitGate::setActive(bool isActive) {
    if (!active && isActive && position.x >= 0.0f) {
        // Celebratory particle burst when summoned/unlocked
        ParticleSystem::getInstance().emitBurst(position, 35, sf::Color(100, 255, 200, 220), 40.0f, 120.0f, 0.4f, 1.0f, 4.0f);
    }
    active = isActive;
}

sf::FloatRect ExitGate::getBounds() const {
    float cellSize = SettingManager::getInstance().getCellSize();
    return sf::FloatRect(
        sf::Vector2f(position.x - cellSize / 2.0f, position.y - cellSize / 2.0f),
        sf::Vector2f(cellSize, cellSize)
    );
}

bool ExitGate::checkPlayerOverlap(const Player& player, float threshold) const {
    if (!active || position.x < 0.0f) return false;

    sf::FloatRect pBounds = player.getBounds();
    sf::FloatRect gBounds = getBounds();

    float left = std::max(pBounds.position.x, gBounds.position.x);
    float top = std::max(pBounds.position.y, gBounds.position.y);
    float right = std::min(pBounds.position.x + pBounds.size.x, gBounds.position.x + gBounds.size.x);
    float bottom = std::min(pBounds.position.y + pBounds.size.y, gBounds.position.y + gBounds.size.y);

    if (right > left && bottom > top) {
        float intersectArea = (right - left) * (bottom - top);
        float playerArea = pBounds.size.x * pBounds.size.y;
        if (playerArea > 0.0f && (intersectArea / playerArea) >= threshold) {
            return true;
        }
    }
    return false;
}

void ExitGate::update(float dt) {
    if (!active || position.x < 0.0f) return;

    pulseTimer += dt;
    particleTimer += dt;

    float pulse = std::sin(pulseTimer * 3.5f);
    uint8_t alpha = static_cast<uint8_t>(std::clamp(80.0f + 40.0f * pulse, 0.0f, 255.0f));
    baseShape.setFillColor(sf::Color(80, 240, 160, alpha));

    float cellSize = SettingManager::getInstance().getCellSize();
    float innerScale = 0.6f + 0.1f * pulse;
    innerShape.setSize(sf::Vector2f(cellSize * innerScale, cellSize * innerScale));
    innerShape.setOrigin(sf::Vector2f(cellSize * innerScale * 0.5f, cellSize * innerScale * 0.5f));

    if (particleTimer >= 0.15f) {
        particleTimer = 0.0f;
        ParticleSystem::getInstance().emitSparkle(position, 2, sf::Color(120, 255, 200, 180), cellSize * 0.45f);
    }
}

void ExitGate::draw(sf::RenderWindow& window) {
    if (!active || position.x < 0.0f) return;

    float cellSize = SettingManager::getInstance().getCellSize();
    AuraRenderer::getInstance().drawAura(window, position, cellSize * 0.85f, sf::Color(80, 240, 180), sf::Color(20, 100, 70), 0.6f);

    window.draw(baseShape);
    window.draw(innerShape);
}
