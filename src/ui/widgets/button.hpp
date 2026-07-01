#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "../base/component.hpp"
#include "../base/text.hpp"
#include "../base/SetterMixin.hpp"
#include <functional>
#include <memory>

namespace UI {

enum class ButtonState {
    Normal,
    Hovered,
    Pressed,
    Disabled
};

// Clickable button widget rendering background shape and a nested Text label
class Button : public Component, public SetterMixin<Button> {
public:
    using SetterMixin<Button>::setModeX;
    using SetterMixin<Button>::setModeY;
    using SetterMixin<Button>::setFixedWidth;
    using SetterMixin<Button>::setFixedHeight;
    using SetterMixin<Button>::setFixedSize;
    using SetterMixin<Button>::setMarginTop;
    using SetterMixin<Button>::setMarginBottom;
    using SetterMixin<Button>::setMarginLeft;
    using SetterMixin<Button>::setMarginRight;
    using SetterMixin<Button>::setMargins;
    using SetterMixin<Button>::setMargin;

    Button(const sf::String& labelText, std::string_view fontKey, unsigned int characterSize = 16);
    virtual ~Button();

    void draw(sf::RenderTarget& target) const override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void onColorPaletteChanged(const ColorPalette& palette) override;
    void computeSize(sf::Vector2f availableSize) override;
    void setPosition(sf::Vector2f pos) override;

    Button* setOnClick(std::function<void()> callback);
    Button* setOnPressed(std::function<void()> callback);

    Button* setLabelText(const sf::String& labelText);
    sf::String getLabelText() const;

    ButtonState getState() const;
    Button* setState(ButtonState newState);
    bool isEnabled() const;
    Button* setEnabled(bool enable);

private:
    ButtonState state;
    std::unique_ptr<Text> label;
    sf::RectangleShape backgroundShape;
    
    std::function<void()> onClickCallback;
    std::function<void()> onPressedCallback;

    bool isInside(sf::Vector2f point) const;
    void updateStyle();
};

} // namespace UI

#endif // BUTTON_HPP
