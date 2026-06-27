#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include "../base/component.hpp"
#include <vector>
#include <memory>

namespace UI {

enum class AlignmentX {
    Left,
    Center,
    Right
};

enum class AlignmentY {
    Top,
    Middle,
    Bottom
};

// Container class managing a collection of children and resolving layout constraints
class Container : public Component {
public:
    Container();
    virtual ~Container();

    void draw(sf::RenderTarget& target) const override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void onColorPaletteChanged(const ColorPalette& palette) override;
    void computeSize(sf::Vector2f availableSize) override;
    void setPosition(sf::Vector2f pos) override;

    Component* addChild(std::unique_ptr<Component> child);

    template <typename T, typename... Args>
    T* createChild(Args&&... args);

    const std::vector<std::unique_ptr<Component>>& getChildren() const;

    float getPaddingTop() const;
    void setPaddingTop(float padding);
    float getPaddingBottom() const;
    void setPaddingBottom(float padding);
    float getPaddingLeft() const;
    void setPaddingLeft(float padding);
    float getPaddingRight() const;
    void setPaddingRight(float padding);
    void setPadding(float top, float bottom, float left, float right);

    AlignmentX getAlignmentX() const;
    void setAlignmentX(AlignmentX align);
    AlignmentY getAlignmentY() const;
    void setAlignmentY(AlignmentY align);

    void setRoot(bool isRoot);
    bool isRoot() const;

protected:
    std::vector<std::unique_ptr<Component>> children;
    
    float paddingTop;
    float paddingBottom;
    float paddingLeft;
    float paddingRight;
    
    AlignmentX alignmentX;
    AlignmentY alignmentY;
    
    bool isRootNode;
};

} // namespace UI

#include "container.tpp"

#endif // CONTAINER_HPP
