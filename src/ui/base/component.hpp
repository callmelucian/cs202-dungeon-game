#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <SFML/Graphics.hpp>
#include "../../global-settings/color-palette-manager.hpp"

namespace ui {

class Component : public sf::Drawable, public ColorPaletteObserver {
public:
    Component();
    virtual ~Component();

    Component(const Component&) = delete;
    Component& operator=(const Component&) = delete;

    virtual void setPosition(const sf::Vector2f& pos);
    virtual const sf::Vector2f& getPosition() const;

    virtual void setSize(const sf::Vector2f& sz);
    virtual const sf::Vector2f& getSize() const;

    virtual sf::FloatRect getGlobalBounds() const;

    virtual bool handleEvent(const sf::Event& event) = 0;
    
    virtual void update(float deltaTime);

    void onColorPaletteChanged(const ColorPalette& newPalette) override;

protected:
    virtual void updateColors(const ColorPalette& palette) = 0;

    sf::Vector2f position;
    sf::Vector2f size;
};

} // namespace ui

#endif // COMPONENT_HPP
