#ifndef TEXT_INPUT_HPP
#define TEXT_INPUT_HPP

#include "../base/component.hpp"
#include "../base/text.hpp"
#include <functional>
#include <string>

namespace UI {

class TextInput : public Component {
public:
    TextInput(const std::string& placeholderText, const sf::Font& font, const sf::Vector2f& size = {300.0f, 40.0f});

    void setPosition(const sf::Vector2f& pos) override;
    void setSize(const sf::Vector2f& sz) override;

    void setPlaceholder(const std::string& placeholderText);
    void setContent(const std::string& text);
    const std::string& getContent() const;

    void setOnTextChanged(const std::function<void(const std::string&)>& callback);
    void setOnEnterPressed(const std::function<void()>& callback);
    void setMaxLength(size_t len);

    bool isFocusedInput() const;
    void setFocused(bool focus);

    void update(float deltaTime) override;
    bool handleEvent(const sf::Event& event) override;

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    void updateColors(const ColorPalette& palette) override;

private:
    void updateLayout();
    void updateVisuals();

    sf::RectangleShape backgroundShape;
    UI::Text textDisplay;
    UI::Text placeholderDisplay;
    sf::RectangleShape cursorLine;

    std::string content;
    std::string placeholder;
    size_t maxLength;
    bool isFocused;
    
    float cursorBlinkTimer;
    bool showCursor;

    std::function<void(const std::string&)> onTextChanged;
    std::function<void()> onEnterPressed;

    sf::Color bgColor;
    sf::Color normalOutlineColor;
    sf::Color focusedOutlineColor;
    sf::Color textColor;
    sf::Color placeholderColor;
    sf::Color cursorColor;
};

} // namespace ui

#endif // TEXT_INPUT_HPP
