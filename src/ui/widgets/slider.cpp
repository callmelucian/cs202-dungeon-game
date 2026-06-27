#include "slider.hpp"
#include <algorithm>

namespace UI {

Slider::Slider(float minValue, float maxValue, float initialValue)
    : minValue(minValue),
      maxValue(maxValue),
      value(initialValue),
      dragging(false),
      dragOffset(0.f) {
    modeX = SizeMode::Fixed;
    modeY = SizeMode::Fixed;
    fixedWidth = 150.f;
    fixedHeight = 20.f;

    trackShape.setFillColor(sf::Color(100, 100, 100));
    
    // Set colors using manager palette
    const auto& palette = ColorPaletteManager::getInstance().getPalette();
    trackShape.setFillColor(palette.disabled);
    thumbShape.setFillColor(palette.primary);
    thumbShape.setOutlineColor(palette.secondary);
    thumbShape.setOutlineThickness(1.f);
}

Slider::~Slider() {
}

void Slider::draw(sf::RenderTarget& target) const {
    target.draw(trackShape);
    target.draw(thumbShape);
}

void Slider::handleEvent(const sf::Event& event) {
    float thumbWidth = 12.f;
    float T = size.x - thumbWidth;
    if (T <= 0.f) T = 1.f;

    if (const auto* mouseButton = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseButton->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos(static_cast<float>(mouseButton->position.x), static_cast<float>(mouseButton->position.y));
            if (isPointInThumb(mousePos)) {
                dragging = true;
                float thumbX = position.x + getThumbPixelOffset();
                dragOffset = mousePos.x - thumbX;
            }
        }
    }

    if (const auto* mouseButton = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseButton->button == sf::Mouse::Button::Left) {
            dragging = false;
        }
    }

    if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        if (dragging) {
            float mouseX = static_cast<float>(mouseMoved->position.x);
            float desiredThumbX = mouseX - dragOffset;
            float localOffset = desiredThumbX - position.x;
            if (localOffset < 0.f) localOffset = 0.f;
            if (localOffset > T) localOffset = T;

            float newValue = getValueFromOffset(localOffset);
            setValue(newValue);
        }
    }
}

void Slider::update(float) {
    // Sliders don't update dynamically per frame by default
}

void Slider::onColorPaletteChanged(const ColorPalette& palette) {
    trackShape.setFillColor(palette.disabled);
    thumbShape.setFillColor(palette.primary);
    thumbShape.setOutlineColor(palette.secondary);
}

void Slider::computeSize(sf::Vector2f availableSize) {
    if (modeX == SizeMode::Fixed) {
        size.x = fixedWidth;
    }
    else if (modeX == SizeMode::Expanded) {
        size.x = availableSize.x - marginLeft - marginRight;
    }
    else {
        size.x = 0.f;
    }

    if (modeY == SizeMode::Fixed) {
        size.y = fixedHeight;
    }
    else if (modeY == SizeMode::Expanded) {
        size.y = availableSize.y - marginTop - marginBottom;
    }
    else {
        size.y = 0.f;
    }

    float trackHeight = 4.f;
    trackShape.setSize(sf::Vector2f(size.x, trackHeight));
    
    float thumbWidth = 12.f;
    thumbShape.setSize(sf::Vector2f(thumbWidth, size.y));
}

void Slider::setPosition(sf::Vector2f pos) {
    position = pos;
    float trackHeight = 4.f;
    trackShape.setPosition(sf::Vector2f(position.x, position.y + (size.y - trackHeight) / 2.f));
    
    float thumbX = position.x + getThumbPixelOffset();
    thumbShape.setPosition(sf::Vector2f(thumbX, position.y));
}

float Slider::getValue() const {
    return value;
}

void Slider::setValue(float newValue) {
    float clampedValue = std::max(minValue, std::min(maxValue, newValue));
    if (value != clampedValue) {
        value = clampedValue;
        
        // Update thumb position immediately
        float thumbX = position.x + getThumbPixelOffset();
        thumbShape.setPosition(sf::Vector2f(thumbX, position.y));
        
        if (onValueChangedCallback) {
            onValueChangedCallback(value);
        }
    }
}

float Slider::getMinValue() const {
    return minValue;
}

float Slider::getMaxValue() const {
    return maxValue;
}

void Slider::setRange(float minVal, float maxVal) {
    minValue = minVal;
    maxValue = maxVal;
    setValue(value); // Re-clamp and trigger changes if necessary
}

void Slider::setOnValueChanged(std::function<void(float)> callback) {
    onValueChangedCallback = callback;
}

float Slider::getThumbPixelOffset() const {
    float thumbWidth = 12.f;
    float T = size.x - thumbWidth;
    if (T <= 0.f) return 0.f;

    float range = maxValue - minValue;
    if (range <= 0.f) return 0.f;

    return (value - minValue) / range * T;
}

float Slider::getValueFromOffset(float pixelOffset) const {
    float thumbWidth = 12.f;
    float T = size.x - thumbWidth;
    if (T <= 0.f) return minValue;

    float range = maxValue - minValue;
    return minValue + (pixelOffset / T) * range;
}

bool Slider::isPointInThumb(sf::Vector2f point) const {
    float thumbWidth = 12.f;
    float thumbX = position.x + getThumbPixelOffset();
    return point.x >= thumbX && point.x <= thumbX + thumbWidth &&
           point.y >= position.y && point.y <= position.y + size.y;
}

} // namespace UI
