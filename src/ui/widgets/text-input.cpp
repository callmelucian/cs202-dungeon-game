#include "text-input.hpp"
#include <cmath>
#include <algorithm>

namespace UI {

TextInput::TextInput(std::string_view fontKey, unsigned int characterSize, const sf::String& defaultText)
    : textDisplay(std::make_unique<Text>(fontKey, characterSize)),
      cursorIndex(0),
      hasFocus(false),
      blinkTimer(0.f),
      blinkDuration(0.53f),
      cursorVisible(false) {
    textDisplay->setString(defaultText);
    textDisplay->setParent(this);
    cursorIndex = defaultText.getSize();

    modeX = SizeMode::Fixed;
    modeY = SizeMode::Fixed;
    fixedWidth = 200.f;
    fixedHeight = 30.f;

    cursorShape.setSize(sf::Vector2f(1.5f, static_cast<float>(characterSize)));
    
    // Set initial colors
    const auto& palette = ColorPaletteManager::getInstance().getPalette();
    backgroundShape.setFillColor(palette.background);
    backgroundShape.setOutlineColor(palette.primary);
    backgroundShape.setOutlineThickness(1.5f);
    cursorShape.setFillColor(palette.text);
}

TextInput::~TextInput() {
}

void TextInput::draw(sf::RenderTarget& target) const {
    target.draw(backgroundShape);
    textDisplay->draw(target);
    if (hasFocus && cursorVisible) {
        target.draw(cursorShape);
    }
}

void TextInput::handleEvent(const sf::Event& event) {
    if (const auto* mouseButton = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseButton->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos(static_cast<float>(mouseButton->position.x), static_cast<float>(mouseButton->position.y));
            if (isInside(mousePos)) {
                setKeyboardFocus(true);
                float localX = mousePos.x - textDisplay->getPosition().x;
                cursorIndex = getIndexFromX(localX);
                updateCursorPosition();
            }
            else {
                setKeyboardFocus(false);
            }
        }
    }

    if (!hasFocus) return;

    if (const auto* textEntered = event.getIf<sf::Event::TextEntered>()) {
        char32_t unicode = textEntered->unicode;
        if (unicode >= 32 && unicode != 127) {
            sf::String currentStr = textDisplay->getString();
            sf::String newStr;
            if (cursorIndex >= currentStr.getSize()) {
                newStr = currentStr + unicode;
            }
            else {
                newStr = currentStr.substring(0, cursorIndex) + unicode + currentStr.substring(cursorIndex);
            }
            textDisplay->setString(newStr);
            cursorIndex++;
            updateCursorPosition();
            
            if (onTextChangedCallback) {
                onTextChangedCallback(newStr);
            }
        }
    }

    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        sf::String currentStr = textDisplay->getString();
        if (keyPressed->code == sf::Keyboard::Key::Left) {
            if (cursorIndex > 0) {
                cursorIndex--;
                updateCursorPosition();
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Right) {
            if (cursorIndex < currentStr.getSize()) {
                cursorIndex++;
                updateCursorPosition();
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Home) {
            cursorIndex = 0;
            updateCursorPosition();
        }
        else if (keyPressed->code == sf::Keyboard::Key::End) {
            cursorIndex = currentStr.getSize();
            updateCursorPosition();
        }
        else if (keyPressed->code == sf::Keyboard::Key::Backspace) {
            if (cursorIndex > 0 && currentStr.getSize() > 0) {
                sf::String newStr = currentStr.substring(0, cursorIndex - 1) + currentStr.substring(cursorIndex);
                textDisplay->setString(newStr);
                cursorIndex--;
                updateCursorPosition();
                if (onTextChangedCallback) {
                    onTextChangedCallback(newStr);
                }
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Delete) {
            if (cursorIndex < currentStr.getSize() && currentStr.getSize() > 0) {
                sf::String newStr = currentStr.substring(0, cursorIndex) + currentStr.substring(cursorIndex + 1);
                textDisplay->setString(newStr);
                updateCursorPosition();
                if (onTextChangedCallback) {
                    onTextChangedCallback(newStr);
                }
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Enter) {
            if (onEnterPressedCallback) {
                onEnterPressedCallback();
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Escape) {
            setKeyboardFocus(false);
        }
    }
}

void TextInput::update(float dt) {
    if (hasFocus) {
        blinkTimer += dt;
        if (blinkTimer >= blinkDuration) {
            blinkTimer -= blinkDuration;
            cursorVisible = !cursorVisible;
        }
    }
    else {
        cursorVisible = false;
    }
}

void TextInput::onColorPaletteChanged(const ColorPalette& palette) {
    textDisplay->onColorPaletteChanged(palette);
    backgroundShape.setFillColor(palette.background);
    backgroundShape.setOutlineColor(hasFocus ? palette.secondary : palette.primary);
    cursorShape.setFillColor(palette.text);
}

void TextInput::computeSize(sf::Vector2f availableSize) {
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

    backgroundShape.setSize(size);
    
    float pad = 5.f;
    textDisplay->computeSize(sf::Vector2f(size.x - 2.f * pad, size.y - 2.f * pad));
}

void TextInput::setPosition(sf::Vector2f pos) {
    position = pos;
    backgroundShape.setPosition(position);
    
    float pad = 5.f;
    sf::Vector2f textDisplaySize = textDisplay->getSize();
    sf::Vector2f textPos(position.x + pad, position.y + (size.y - textDisplaySize.y) / 2.f);
    textDisplay->setPosition(textPos);
    
    updateCursorPosition();
}

TextInput* TextInput::setTextString(const sf::String& str) {
    textDisplay->setString(str);
    cursorIndex = str.getSize();
    updateCursorPosition();
    return this;
}

sf::String TextInput::getTextString() const {
    return textDisplay->getString();
}

bool TextInput::hasKeyboardFocus() const {
    return hasFocus;
}

TextInput* TextInput::setKeyboardFocus(bool focus) {
    if (hasFocus != focus) {
        hasFocus = focus;
        blinkTimer = 0.f;
        cursorVisible = hasFocus;
        
        // Update styling instantly
        const auto& palette = ColorPaletteManager::getInstance().getPalette();
        backgroundShape.setOutlineColor(hasFocus ? palette.secondary : palette.primary);
    }
    return this;
}

TextInput* TextInput::setOnTextChanged(std::function<void(const sf::String&)> callback) {
    onTextChangedCallback = callback;
    return this;
}

TextInput* TextInput::setOnEnterPressed(std::function<void()> callback) {
    onEnterPressedCallback = callback;
    return this;
}

TextInput* TextInput::setBlinkDuration(float durationInSeconds) {
    blinkDuration = durationInSeconds;
    return this;
}

float TextInput::getBlinkDuration() const {
    return blinkDuration;
}

std::uint32_t TextInput::getIndexFromX(float xOffset) const {
    const sf::Text& sfText = textDisplay->getUnderlyingText();
    std::uint32_t length = sfText.getString().getSize();
    std::uint32_t bestIndex = 0;
    float minDiff = 999999.f;
    for (std::uint32_t i = 0; i <= length; i++) {
        float charX = sfText.findCharacterPos(i).x;
        float diff = std::abs(charX - xOffset);
        if (diff < minDiff) {
            minDiff = diff;
            bestIndex = i;
        }
    }
    return bestIndex;
}

void TextInput::updateCursorPosition() {
    const sf::Text& sfText = textDisplay->getUnderlyingText();
    sf::Vector2f localCursorPos = sfText.findCharacterPos(cursorIndex);
    sf::Vector2f textPos = textDisplay->getPosition();
    cursorShape.setPosition(sf::Vector2f(textPos.x + localCursorPos.x, textPos.y + localCursorPos.y));
    blinkTimer = 0.f;
    cursorVisible = true;
}

bool TextInput::isInside(sf::Vector2f point) const {
    return point.x >= position.x && point.x <= position.x + size.x &&
           point.y >= position.y && point.y <= position.y + size.y;
}

} // namespace UI
