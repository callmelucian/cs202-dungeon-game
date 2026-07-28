#include "enemy-health-bar.hpp"

namespace UI {

EnemyHealthBar::EnemyHealthBar() : HealthBar() {
    fixedWidth = 58.f * barScale.x;
    fixedHeight = 7.f * barScale.y;
    size = sf::Vector2f(fixedWidth, fixedHeight);
    updateSprites();
}

void EnemyHealthBar::updateSprites() {
    try {
        const sf::Texture& texture = AssetManager::getInstance().getTexture("health-bar");

        float scaleX = (size.x > 0.f) ? (size.x / 58.f) : barScale.x;
        float scaleY = (size.y > 0.f) ? (size.y / 7.f) : barScale.y;
        sf::Vector2f scale(scaleX, scaleY);

        // 1. Outline at (0, 0), size (58, 7), texture rect (27, 71, 58, 7)
        outlineSprite.emplace(texture, sf::IntRect({27, 71}, {58, 7}));
        outlineSprite->setPosition(position + sf::Vector2f(0.f * scaleX, 0.f * scaleY));
        outlineSprite->setScale(scale);

        // 2. Health fill at (1, 1), max width 56, height 4, texture rect (28, 61, 56 * displayedHealthRatio, 4)
        int fillWidth = static_cast<int>(std::round(56.f * displayedHealthRatio));
        healthFillSprite.emplace(texture, sf::IntRect({28, 61}, {fillWidth, 4}));
        healthFillSprite->setPosition(position + sf::Vector2f(1.f * scaleX, 1.f * scaleY));
        healthFillSprite->setScale(scale);
    } catch (const std::exception&) {
        // Texture not loaded yet
    }
}

} // namespace UI
