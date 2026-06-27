#ifndef TEXT_INPUT_HPP
#define TEXT_INPUT_HPP

#include "../base/component.hpp"
#include "../base/text.hpp"
#include <functional>
#include <memory>

namespace UI {

// Single-line editable text input widget with blinking keyboard cursor
class TextInput : public Component {
public:
    TextInput(std::string_view fontKey, unsigned int characterSize = 16, const sf::String& defaultText = "");
    virtual ~TextInput();

    void draw(sf::RenderTarget& target) const override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void onColorPaletteChanged(const ColorPalette& palette) override;
    void computeSize(sf::Vector2f availableSize) override;
    void setPosition(sf::Vector2f pos) override;

    void setTextString(const sf::String& str);
    sf::String getTextString() const;

    bool hasKeyboardFocus() const;
    void setKeyboardFocus(bool focus);

    void setOnTextChanged(std::function<void(const sf::String&)> callback);
    void setOnEnterPressed(std::function<void()> callback);

    void setBlinkDuration(float durationInSeconds);
    float getBlinkDuration() const;

private:
    std::unique_ptr<Text> textDisplay;
    sf::RectangleShape backgroundShape;
    sf::RectangleShape cursorShape;
    
    std::uint32_t cursorIndex;
    bool hasFocus;
    
    float blinkTimer;
    float blinkDuration;
    bool cursorVisible;

    std::function<void(const sf::String&)> onTextChangedCallback;
    std::function<void()> onEnterPressedCallback;

    std::uint32_t getIndexFromX(float xOffset) const;
    void updateCursorPosition();
    bool isInside(sf::Vector2f point) const;
};

} // namespace UI

#endif // TEXT_INPUT_HPP
