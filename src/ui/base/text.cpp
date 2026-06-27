#include "text.hpp"
#include "../../global-settings/asset-manager.hpp"

namespace UI {

Text::Text(std::string_view fontKey, unsigned int characterSize)
    : fontKey(fontKey),
      text(AssetManager::getInstance().getFont(std::string(fontKey)), "", characterSize) {
    text.setFillColor(ColorPaletteManager::getInstance().getPalette().text);
    modeX = SizeMode::Fixed;
    modeY = SizeMode::Fixed;
}

Text::~Text() {
}

void Text::draw(sf::RenderTarget& target) const {
    target.draw(text);
}

void Text::handleEvent(const sf::Event&) {
    // Text doesn't respond to events by default
}

void Text::update(float) {
    // Text doesn't update dynamically per frame by default
}

void Text::onColorPaletteChanged(const ColorPalette& palette) {
    text.setFillColor(palette.text);
}

void Text::computeSize(sf::Vector2f availableSize) {
    sf::FloatRect bounds = text.getLocalBounds();

    if (modeX == SizeMode::Fixed) {
        size.x = (fixedWidth > 0.f) ? fixedWidth : bounds.size.x;
    }
    else if (modeX == SizeMode::Expanded) {
        size.x = availableSize.x - marginLeft - marginRight;
    }
    else {
        size.x = 0.f;
    }

    if (modeY == SizeMode::Fixed) {
        size.y = (fixedHeight > 0.f) ? fixedHeight : bounds.size.y;
    }
    else if (modeY == SizeMode::Expanded) {
        size.y = availableSize.y - marginTop - marginBottom;
    }
    else {
        size.y = 0.f;
    }
}

void Text::setPosition(sf::Vector2f pos) {
    position = pos;
    sf::FloatRect bounds = text.getLocalBounds();
    text.setPosition(position - bounds.position);
}

void Text::setString(const sf::String& str) {
    text.setString(str);
}

const sf::String& Text::getString() const {
    return text.getString();
}

void Text::setCharacterSize(unsigned int size) {
    text.setCharacterSize(size);
}

unsigned int Text::getCharacterSize() const {
    return text.getCharacterSize();
}

void Text::setLineSpacing(float spacingFactor) {
    text.setLineSpacing(spacingFactor);
}

float Text::getLineSpacing() const {
    return text.getLineSpacing();
}

void Text::setLetterSpacing(float spacingFactor) {
    text.setLetterSpacing(spacingFactor);
}

float Text::getLetterSpacing() const {
    return text.getLetterSpacing();
}

void Text::setStyle(std::uint32_t style) {
    text.setStyle(style);
}

std::uint32_t Text::getStyle() const {
    return text.getStyle();
}

void Text::setFillColor(const sf::Color& color) {
    text.setFillColor(color);
}

sf::Color Text::getFillColor() const {
    return text.getFillColor();
}

void Text::setOutlineColor(const sf::Color& color) {
    text.setOutlineColor(color);
}

sf::Color Text::getOutlineColor() const {
    return text.getOutlineColor();
}

void Text::setOutlineThickness(float thickness) {
    text.setOutlineThickness(thickness);
}

float Text::getOutlineThickness() const {
    return text.getOutlineThickness();
}

const sf::Text& Text::getUnderlyingText() const {
    return text;
}

} // namespace UI
