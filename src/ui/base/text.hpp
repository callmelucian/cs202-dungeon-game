#ifndef TEXT_HPP
#define TEXT_HPP

#include "component.hpp"
#include <string>
#include <string_view>

namespace UI {

// Wrapper component around sf::Text sourcing fonts dynamically from AssetManager
class Text : public Component {
public:
    explicit Text(std::string_view fontKey, unsigned int characterSize = 16);
    virtual ~Text();

    void draw(sf::RenderTarget& target) const override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void onColorPaletteChanged(const ColorPalette& palette) override;
    void computeSize(sf::Vector2f availableSize) override;
    void setPosition(sf::Vector2f pos) override;

    void setString(const sf::String& str);
    const sf::String& getString() const;

    void setCharacterSize(unsigned int size);
    unsigned int getCharacterSize() const;

    void setLineSpacing(float spacingFactor);
    float getLineSpacing() const;

    void setLetterSpacing(float spacingFactor);
    float getLetterSpacing() const;

    void setStyle(std::uint32_t style);
    std::uint32_t getStyle() const;

    void setFillColor(const sf::Color& color);
    sf::Color getFillColor() const;

    void setOutlineColor(const sf::Color& color);
    sf::Color getOutlineColor() const;

    void setOutlineThickness(float thickness);
    float getOutlineThickness() const;

    const sf::Text& getUnderlyingText() const;

private:
    sf::Text text;
    std::string fontKey;
};

} // namespace UI

#endif // TEXT_HPP
