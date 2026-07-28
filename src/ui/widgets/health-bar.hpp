#ifndef HEALTH_BAR_HPP
#define HEALTH_BAR_HPP

#include "../base/component.hpp"
#include "../base/SetterMixin.hpp"
#include "../../global-settings/asset-manager.hpp"
#include <algorithm>
#include <optional>

namespace UI {

// Abstract base class for UI HealthBars (PlayerHealthBar, EnemyHealthBar)
class HealthBar : public Component, public SetterMixin<HealthBar> {
public:
    using SetterMixin<HealthBar>::setModeX;
    using SetterMixin<HealthBar>::setModeY;
    using SetterMixin<HealthBar>::setFixedWidth;
    using SetterMixin<HealthBar>::setFixedHeight;
    using SetterMixin<HealthBar>::setFixedSize;
    using SetterMixin<HealthBar>::setMarginTop;
    using SetterMixin<HealthBar>::setMarginBottom;
    using SetterMixin<HealthBar>::setMarginLeft;
    using SetterMixin<HealthBar>::setMarginRight;
    using SetterMixin<HealthBar>::setMargins;
    using SetterMixin<HealthBar>::setMargin;

    HealthBar();
    virtual ~HealthBar() = default;

    void draw(sf::RenderTarget& target) const override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void onColorPaletteChanged(const ColorPalette& palette) override;
    void computeSize(sf::Vector2f availableSize) override;
    void setPosition(sf::Vector2f pos) override;

    HealthBar* setHealthRatio(float ratio, bool immediate = false);
    float getHealthRatio() const;
    float getDisplayedHealthRatio() const;
    HealthBar* setHealth(float current, float max, bool immediate = false);

    HealthBar* setMomentumRatio(float ratio, bool immediate = false);
    float getMomentumRatio() const;
    float getDisplayedMomentumRatio() const;
    HealthBar* setMomentum(float current, float max, bool immediate = false);

    HealthBar* setLerpSpeed(float speed);
    float getLerpSpeed() const;

    HealthBar* setBarScale(float scale);
    HealthBar* setBarScale(sf::Vector2f scale);
    sf::Vector2f getBarScale() const;

    HealthBar* snapToTarget();

    HealthBar* setTogglerVisible(bool visible);
    bool isTogglerVisible() const;

protected:
    float targetHealthRatio;
    float displayedHealthRatio;

    float targetMomentumRatio;
    float displayedMomentumRatio;

    float lerpSpeed;
    sf::Vector2f barScale;
    bool showToggler;

    std::optional<sf::Sprite> outlineSprite;
    std::optional<sf::Sprite> healthFillSprite;
    std::optional<sf::Sprite> momentumFillSprite;
    std::optional<sf::Sprite> togglerSprite;

    virtual void updateSprites() = 0;
};

} // namespace UI

#endif // HEALTH_BAR_HPP
