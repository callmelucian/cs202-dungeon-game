#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "../base/component.hpp"
#include "../base/text.hpp"
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
class Button : public Component {
public:
    Button(const sf::String& labelText, std::string_view fontKey, unsigned int characterSize = 16);
    virtual ~Button();

    void draw(sf::RenderTarget& target) const override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void onColorPaletteChanged(const ColorPalette& palette) override;
    void computeSize(sf::Vector2f availableSize) override;
    void setPosition(sf::Vector2f pos) override;

    void setOnClick(std::function<void()> callback);
    void setOnPressed(std::function<void()> callback);

    void setLabelText(const sf::String& labelText);
    sf::String getLabelText() const;

    ButtonState getState() const;
    void setState(ButtonState newState);
    bool isEnabled() const;
    void setEnabled(bool enable);

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
