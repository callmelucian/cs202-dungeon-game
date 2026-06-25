#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "../base/component.hpp"
#include "../base/text.hpp"
#include <functional>
#include <string>

namespace UI {

class Button : public Component {
public:
    Button(const std::string& labelText, const sf::Font& font, const sf::Vector2f& size = {200.0f, 50.0f});

    void setPosition(const sf::Vector2f& pos) override;
    void setSize(const sf::Vector2f& sz) override;

    void setOnClick(const std::function<void()>& callback);
    void setString(const std::string& labelText);
    std::string getString() const;

    bool handleEvent(const sf::Event& event) override;

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    void updateColors(const ColorPalette& palette) override;

private:
    void centerText();
    void updateVisuals();

    UI::Text buttonText;
    sf::RectangleShape backgroundShape;
    std::function<void()> onClick;
    
    bool isHovered;
    bool isPressed;

    sf::Color normalColor;
    sf::Color hoverColor;
    sf::Color pressedColor;
    sf::Color textColor;
};

} // namespace ui

#endif // BUTTON_HPP
