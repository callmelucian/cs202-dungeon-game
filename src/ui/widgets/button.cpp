#include "button.hpp"

namespace UI {

Button::Button(const sf::String& labelText, std::string_view fontKey, unsigned int characterSize)
    : state(ButtonState::Normal),
      label(std::make_unique<Text>(fontKey, characterSize)) {
    label->setString(labelText);
    label->setParent(this);
    modeX = SizeMode::Fixed;
    modeY = SizeMode::Fixed;
    fixedWidth = 100.f;
    fixedHeight = 40.f;

    backgroundShape.setOutlineThickness(2.f);
    
    // Set initial colors
    const auto& palette = ColorPaletteManager::getInstance().getPalette();
    backgroundShape.setFillColor(palette.background);
    backgroundShape.setOutlineColor(palette.primary);
    label->setFillColor(palette.text);
}

Button::~Button() {
}

void Button::draw(sf::RenderTarget& target) const {
    target.draw(backgroundShape);
    label->draw(target);
}

void Button::handleEvent(const sf::Event& event) {
    if (state == ButtonState::Disabled) return;

    if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mousePos(static_cast<float>(mouseMoved->position.x), static_cast<float>(mouseMoved->position.y));
        if (isInside(mousePos)) {
            if (state == ButtonState::Normal) {
                state = ButtonState::Hovered;
                updateStyle();
            }
        }
        else {
            if (state == ButtonState::Hovered) {
                state = ButtonState::Normal;
                updateStyle();
            }
        }
    }

    if (const auto* mouseButton = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseButton->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos(static_cast<float>(mouseButton->position.x), static_cast<float>(mouseButton->position.y));
            if (isInside(mousePos)) {
                state = ButtonState::Pressed;
                updateStyle();
                if (onPressedCallback) {
                    onPressedCallback();
                }
            }
        }
    }

    if (const auto* mouseButton = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseButton->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos(static_cast<float>(mouseButton->position.x), static_cast<float>(mouseButton->position.y));
            if (state == ButtonState::Pressed) {
                if (isInside(mousePos)) {
                    state = ButtonState::Hovered;
                    updateStyle();
                    if (onClickCallback) {
                        onClickCallback();
                    }
                }
                else {
                    state = ButtonState::Normal;
                    updateStyle();
                }
            }
        }
    }
}

void Button::update(float) {
    // Buttons don't need real-time frame updates by default
}

void Button::onColorPaletteChanged(const ColorPalette&) {
    updateStyle();
}

void Button::computeSize(sf::Vector2f availableSize) {
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
    label->computeSize(size);
}

void Button::setPosition(sf::Vector2f pos) {
    position = pos;
    backgroundShape.setPosition(position);
    
    sf::Vector2f labelSize = label->getSize();
    sf::Vector2f labelPos = position + (size - labelSize) / 2.f;
    label->setPosition(labelPos);
}

Button* Button::setOnClick(std::function<void()> callback) {
    onClickCallback = callback;
    return this;
}

Button* Button::setOnPressed(std::function<void()> callback) {
    onPressedCallback = callback;
    return this;
}

Button* Button::setLabelText(const sf::String& labelText) {
    label->setString(labelText);
    label->computeSize(size);
    // Center it again after string changes
    sf::Vector2f labelSize = label->getSize();
    sf::Vector2f labelPos = position + (size - labelSize) / 2.f;
    label->setPosition(labelPos);
    return this;
}

sf::String Button::getLabelText() const {
    return label->getString();
}

ButtonState Button::getState() const {
    return state;
}

Button* Button::setState(ButtonState newState) {
    state = newState;
    updateStyle();
    return this;
}

bool Button::isEnabled() const {
    return state != ButtonState::Disabled;
}

Button* Button::setEnabled(bool enable) {
    if (enable) {
        if (state == ButtonState::Disabled) {
            state = ButtonState::Normal;
            updateStyle();
        }
    }
    else {
        state = ButtonState::Disabled;
        updateStyle();
    }
    return this;
}

bool Button::isInside(sf::Vector2f point) const {
    return point.x >= position.x && point.x <= position.x + size.x &&
           point.y >= position.y && point.y <= position.y + size.y;
}

void Button::updateStyle() {
    const auto& palette = ColorPaletteManager::getInstance().getPalette();
    if (state == ButtonState::Disabled) {
        backgroundShape.setFillColor(palette.disabled);
        backgroundShape.setOutlineColor(palette.disabled);
        label->setFillColor(sf::Color(128, 128, 128));
    }
    else if (state == ButtonState::Pressed) {
        backgroundShape.setFillColor(palette.accent);
        backgroundShape.setOutlineColor(palette.secondary);
        label->setFillColor(palette.background);
    }
    else if (state == ButtonState::Hovered) {
        backgroundShape.setFillColor(palette.primary);
        backgroundShape.setOutlineColor(palette.secondary);
        label->setFillColor(palette.text);
    }
    else { // Normal
        backgroundShape.setFillColor(palette.background);
        backgroundShape.setOutlineColor(palette.primary);
        label->setFillColor(palette.text);
    }
}

} // namespace UI
