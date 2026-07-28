#include "player-health-bar.hpp"

namespace UI {

PlayerHealthBar::PlayerHealthBar() : HealthBar() {
    fixedWidth = 64.f * barScale.x;
    fixedHeight = 10.f * barScale.y;
    size = sf::Vector2f(fixedWidth, fixedHeight);
    updateSprites();
}

void PlayerHealthBar::updateSprites() {
    try {
        const sf::Texture& texture = AssetManager::getInstance().getTexture("health-bar");

        float scaleX = (size.x > 0.f) ? (size.x / 64.f) : barScale.x;
        float scaleY = (size.y > 0.f) ? (size.y / 10.f) : barScale.y;
        sf::Vector2f scale(scaleX, scaleY);

        // 1. Outline at (0, 0), size (64, 10), texture rect (18, 40, 64, 10)
        outlineSprite.emplace(texture, sf::IntRect({18, 40}, {64, 10}));
        outlineSprite->setPosition(position + sf::Vector2f(0.f * scaleX, 0.f * scaleY));
        outlineSprite->setScale(scale);

        // 2. Toggler at (1, 1), size (4, 4), texture rect (19, 30, 4, 4)
        togglerSprite.emplace(texture, sf::IntRect({19, 30}, {4, 4}));
        togglerSprite->setPosition(position + sf::Vector2f(1.f * scaleX, 1.f * scaleY));
        togglerSprite->setScale(scale);

        // 3. Health fill at (5, 1), max width 58, height 4, texture rect (23, 30, 58 * displayedHealthRatio, 4)
        int fillWidth = static_cast<int>(std::round(58.f * displayedHealthRatio));
        healthFillSprite.emplace(texture, sf::IntRect({23, 30}, {fillWidth, 4}));
        healthFillSprite->setPosition(position + sf::Vector2f(5.f * scaleX, 1.f * scaleY));
        healthFillSprite->setScale(scale);

        // 4. Momentum fill at (2, 6), max width 56, height 2, texture rect (20, 35, 56 * displayedMomentumRatio, 2)
        int momentumWidth = static_cast<int>(std::round(56.f * displayedMomentumRatio));
        momentumFillSprite.emplace(texture, sf::IntRect({20, 35}, {momentumWidth, 2}));
        momentumFillSprite->setPosition(position + sf::Vector2f(2.f * scaleX, 6.f * scaleY));
        momentumFillSprite->setScale(scale);
    } catch (const std::exception&) {
        // Texture not loaded yet
    }
}

} // namespace UI
