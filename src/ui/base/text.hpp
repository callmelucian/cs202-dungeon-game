#ifndef TEXT_HPP
#define TEXT_HPP

#include "component.hpp"
#include "SetterMixin.hpp"
#include <string>
#include <string_view>

namespace UI {

// Wrapper component around sf::Text sourcing fonts dynamically from AssetManager
class Text : public Component, public SetterMixin<Text> {
public:
    using SetterMixin<Text>::setModeX;
    using SetterMixin<Text>::setModeY;
    using SetterMixin<Text>::setFixedWidth;
    using SetterMixin<Text>::setFixedHeight;
    using SetterMixin<Text>::setFixedSize;
    using SetterMixin<Text>::setMarginTop;
    using SetterMixin<Text>::setMarginBottom;
    using SetterMixin<Text>::setMarginLeft;
    using SetterMixin<Text>::setMarginRight;
    using SetterMixin<Text>::setMargins;
    using SetterMixin<Text>::setMargin;

    explicit Text(std::string_view fontKey, unsigned int characterSize = 16);
    virtual ~Text();

    void draw(sf::RenderTarget& target) const override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void onColorPaletteChanged(const ColorPalette& palette) override;
    void computeSize(sf::Vector2f availableSize) override;
    void setPosition(sf::Vector2f pos) override;

    Text* setString(const sf::String& str);
    const sf::String& getString() const;

    Text* setCharacterSize(unsigned int size);
    unsigned int getCharacterSize() const;

    Text* setLineSpacing(float spacingFactor);
    float getLineSpacing() const;

    Text* setLetterSpacing(float spacingFactor);
    float getLetterSpacing() const;

    Text* setStyle(std::uint32_t style);
    std::uint32_t getStyle() const;

    Text* setFillColor(const sf::Color& color);
    sf::Color getFillColor() const;

    Text* setOutlineColor(const sf::Color& color);
    sf::Color getOutlineColor() const;

    Text* setOutlineThickness(float thickness);
    float getOutlineThickness() const;

    const sf::Text& getUnderlyingText() const;

private:
    sf::Text text;
    std::string fontKey;
};

} // namespace UI

#endif // TEXT_HPP
