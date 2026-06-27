#ifndef SLIDER_HPP
#define SLIDER_HPP

#include "../base/component.hpp"
#include <functional>

namespace UI {

// Draggable horizontal track-and-thumb value slider widget
class Slider : public Component {
public:
    Slider(float minValue, float maxValue, float initialValue);
    virtual ~Slider();

    void draw(sf::RenderTarget& target) const override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void onColorPaletteChanged(const ColorPalette& palette) override;
    void computeSize(sf::Vector2f availableSize) override;
    void setPosition(sf::Vector2f pos) override;

    float getValue() const;
    void setValue(float newValue);

    float getMinValue() const;
    float getMaxValue() const;
    void setRange(float minVal, float maxVal);

    void setOnValueChanged(std::function<void(float)> callback);

private:
    float minValue;
    float maxValue;
    float value;

    bool dragging;
    float dragOffset;

    sf::RectangleShape trackShape;
    sf::RectangleShape thumbShape;

    std::function<void(float)> onValueChangedCallback;

    float getThumbPixelOffset() const;
    float getValueFromOffset(float pixelOffset) const;
    bool isPointInThumb(sf::Vector2f point) const;
};

} // namespace UI

#endif // SLIDER_HPP
