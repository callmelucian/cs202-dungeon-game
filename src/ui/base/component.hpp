#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <SFML/Graphics.hpp>
#include "../../global-settings/color-palette-manager.hpp"

namespace UI {

enum class SizeMode {
    Fixed,
    Contained,
    Expanded
};

// Abstract base class representing a UI node in the component hierarchy tree
class Component : public ColorPaletteObserver {
public:
    Component();
    virtual ~Component();

    Component(const Component&) = delete;
    Component& operator=(const Component&) = delete;
    Component(Component&&) = delete;
    Component& operator=(Component&&) = delete;

    virtual void draw(sf::RenderTarget& target) const = 0;
    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(float dt) = 0;
    virtual void onColorPaletteChanged(const ColorPalette& palette) override = 0;
    virtual void computeSize(sf::Vector2f availableSize) = 0;
    virtual void setPosition(sf::Vector2f pos) = 0;

    Component* getParent() const;
    void setParent(Component* newParent);

    sf::Vector2f getPosition() const;
    sf::Vector2f getSize() const;

    SizeMode getModeX() const;
    void setModeX(SizeMode mode);

    SizeMode getModeY() const;
    void setModeY(SizeMode mode);

    float getFixedWidth() const;
    void setFixedWidth(float width);
    float getFixedHeight() const;
    void setFixedHeight(float height);
    void setFixedSize(sf::Vector2f size);

    float getMarginTop() const;
    void setMarginTop(float margin);
    float getMarginBottom() const;
    void setMarginBottom(float margin);
    float getMarginLeft() const;
    void setMarginLeft(float margin);
    float getMarginRight() const;
    void setMarginRight(float margin);
    void setMargins(float top, float bottom, float left, float right);

protected:
    Component* parent;
    sf::Vector2f position;
    sf::Vector2f size;
    
    SizeMode modeX;
    SizeMode modeY;
    
    float fixedWidth;
    float fixedHeight;
    
    float marginTop;
    float marginBottom;
    float marginLeft;
    float marginRight;
};

} // namespace UI

#endif // COMPONENT_HPP
