#ifndef SLIDER_HPP
#define SLIDER_HPP

#include "../base/component.hpp"
#include <functional>

namespace ui {

class Slider : public Component {
public:
    Slider(float minVal = 0.0f, float maxVal = 100.0f, float initialVal = 50.0f, const sf::Vector2f& size = {200.0f, 30.0f});

    void setPosition(const sf::Vector2f& pos) override;
    void setSize(const sf::Vector2f& sz) override;

    void setRange(float minVal, float maxVal);
    void setValue(float val);
    float getValue() const;

    void setOnValueChanged(const std::function<void(float)>& callback);

    bool handleEvent(const sf::Event& event) override;

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    void updateColors(const ColorPalette& palette) override;

private:
    void updateLayout();
    void updateVisuals();

    sf::RectangleShape trackShape;
    sf::RectangleShape filledTrackShape;
    sf::RectangleShape thumbShape;

    float minValue;
    float maxValue;
    float currentValue;
    bool isDragging;
    bool isHovered;

    float trackHeight;
    float thumbWidth;
    float thumbHeight;

    std::function<void(float)> onValueChanged;

    sf::Color trackColor;
    sf::Color filledColor;
    sf::Color thumbNormalColor;
    sf::Color thumbHoverColor;
};

} // namespace ui

#endif // SLIDER_HPP
