#include "health-bar.hpp"
#include <cmath>
#include <iostream>

namespace UI {

HealthBar::HealthBar()
    : targetHealthRatio(1.0f),
      displayedHealthRatio(1.0f),
      targetMomentumRatio(1.0f),
      displayedMomentumRatio(1.0f),
      lerpSpeed(8.0f),
      barScale(0.65f, 0.65f),
      showToggler(true) {
    modeX = SizeMode::Fixed;
    modeY = SizeMode::Fixed;
}

void HealthBar::draw(sf::RenderTarget& target) const {
    if (showToggler && togglerSprite.has_value()) {
        target.draw(*togglerSprite);
    }
    if (displayedHealthRatio > 0.f && healthFillSprite.has_value()) {
        target.draw(*healthFillSprite);
    }
    if (displayedMomentumRatio > 0.f && momentumFillSprite.has_value()) {
        target.draw(*momentumFillSprite);
    }
    if (outlineSprite.has_value()) {
        target.draw(*outlineSprite);
    }
}

void HealthBar::handleEvent(const sf::Event&) {
}

void HealthBar::update(float dt) {
    if (dt <= 0.0f) return;

    bool changed = false;

    if (std::abs(displayedHealthRatio - targetHealthRatio) > 0.0001f) {
        float factor = 1.0f - std::exp(-lerpSpeed * dt);
        displayedHealthRatio += (targetHealthRatio - displayedHealthRatio) * factor;
        if (std::abs(displayedHealthRatio - targetHealthRatio) <= 0.001f) {
            displayedHealthRatio = targetHealthRatio;
        }
        changed = true;
    }

    if (std::abs(displayedMomentumRatio - targetMomentumRatio) > 0.0001f) {
        float factor = 1.0f - std::exp(-lerpSpeed * dt);
        displayedMomentumRatio += (targetMomentumRatio - displayedMomentumRatio) * factor;
        if (std::abs(displayedMomentumRatio - targetMomentumRatio) <= 0.001f) {
            displayedMomentumRatio = targetMomentumRatio;
        }
        changed = true;
    }

    if (changed) {
        updateSprites();
    }
}

void HealthBar::onColorPaletteChanged(const ColorPalette&) {
}

void HealthBar::computeSize(sf::Vector2f availableSize) {
    float defaultW = 64.f * barScale.x;
    float defaultH = 10.f * barScale.y;

    if (modeX == SizeMode::Fixed) {
        size.x = (fixedWidth > 0.f) ? fixedWidth : defaultW;
    } else if (modeX == SizeMode::Expanded) {
        size.x = availableSize.x - marginLeft - marginRight;
    } else {
        size.x = defaultW;
    }

    if (modeY == SizeMode::Fixed) {
        size.y = (fixedHeight > 0.f) ? fixedHeight : defaultH;
    } else if (modeY == SizeMode::Expanded) {
        size.y = availableSize.y - marginTop - marginBottom;
    } else {
        size.y = defaultH;
    }

    updateSprites();
}

void HealthBar::setPosition(sf::Vector2f pos) {
    position = pos;
    updateSprites();
}

HealthBar* HealthBar::setHealthRatio(float ratio, bool immediate) {
    targetHealthRatio = std::clamp(ratio, 0.0f, 1.0f);
    if (immediate) {
        displayedHealthRatio = targetHealthRatio;
    }
    updateSprites();
    return this;
}

float HealthBar::getHealthRatio() const {
    return targetHealthRatio;
}

float HealthBar::getDisplayedHealthRatio() const {
    return displayedHealthRatio;
}

HealthBar* HealthBar::setHealth(float current, float max, bool immediate) {
    if (max <= 0.0f) {
        return setHealthRatio(0.0f, immediate);
    }
    return setHealthRatio(current / max, immediate);
}

HealthBar* HealthBar::setMomentumRatio(float ratio, bool immediate) {
    targetMomentumRatio = std::clamp(ratio, 0.0f, 1.0f);
    if (immediate) {
        displayedMomentumRatio = targetMomentumRatio;
    }
    updateSprites();
    return this;
}

float HealthBar::getMomentumRatio() const {
    return targetMomentumRatio;
}

float HealthBar::getDisplayedMomentumRatio() const {
    return displayedMomentumRatio;
}

HealthBar* HealthBar::setMomentum(float current, float max, bool immediate) {
    if (max <= 0.0f) {
        return setMomentumRatio(0.0f, immediate);
    }
    return setMomentumRatio(current / max, immediate);
}

HealthBar* HealthBar::setLerpSpeed(float speed) {
    lerpSpeed = std::max(0.0f, speed);
    return this;
}

float HealthBar::getLerpSpeed() const {
    return lerpSpeed;
}

HealthBar* HealthBar::setBarScale(float scale) {
    return setBarScale(sf::Vector2f(scale, scale));
}

HealthBar* HealthBar::setBarScale(sf::Vector2f scale) {
    barScale = scale;
    fixedWidth = 64.f * barScale.x;
    fixedHeight = 10.f * barScale.y;
    size = sf::Vector2f(fixedWidth, fixedHeight);
    updateSprites();
    return this;
}

sf::Vector2f HealthBar::getBarScale() const {
    return barScale;
}

HealthBar* HealthBar::snapToTarget() {
    displayedHealthRatio = targetHealthRatio;
    displayedMomentumRatio = targetMomentumRatio;
    updateSprites();
    return this;
}

HealthBar* HealthBar::setTogglerVisible(bool visible) {
    showToggler = visible;
    return this;
}

bool HealthBar::isTogglerVisible() const {
    return showToggler;
}

} // namespace UI
