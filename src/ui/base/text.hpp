#ifndef TEXT_HPP
#define TEXT_HPP

#include <SFML/Graphics.hpp>
#include <string>

namespace ui {

class Text : public sf::Drawable {
public:
    Text();
    Text(const std::string& string, const sf::Font& font, unsigned int characterSize = 30);

    void setString(const std::string& string);
    std::string getString() const;

    void setFont(const sf::Font& font);
    const sf::Font* getFont() const;

    void setCharacterSize(unsigned int size);
    unsigned int getCharacterSize() const;

    void setFillColor(const sf::Color& color);
    const sf::Color& getFillColor() const;

    void setPosition(const sf::Vector2f& position);
    const sf::Vector2f& getPosition() const;

    sf::FloatRect getLocalBounds() const;
    sf::FloatRect getGlobalBounds() const;

    sf::Text& getUnderlyingText();
    const sf::Text& getUnderlyingText() const;

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    sf::Text text;
};

} // namespace ui

#endif // TEXT_HPP
