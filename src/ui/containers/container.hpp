#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include "../base/component.hpp"
#include <vector>
#include <memory>
#include <optional>

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

struct Margin {
    float top = 0.f;
    float bottom = 0.f;
    float left = 0.f;
    float right = 0.f;
};

struct ChildDefaults {
    std::optional<SizeMode> modeX;
    std::optional<SizeMode> modeY;
    std::optional<float>    fixedWidth;
    std::optional<float>    fixedHeight;
    std::optional<Margin>   margin;
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

    Container* setChildDefaults(ChildDefaults defaults);
    Container* clearChildDefaults();
    Container* pushChildDefaults(ChildDefaults defaults);
    Container* popChildDefaults();
    std::optional<ChildDefaults> activeChildDefaults() const;

    float getPaddingTop() const;
    Container* setPaddingTop(float padding);
    float getPaddingBottom() const;
    Container* setPaddingBottom(float padding);
    float getPaddingLeft() const;
    Container* setPaddingLeft(float padding);
    float getPaddingRight() const;
    Container* setPaddingRight(float padding);
    Container* setPadding(float top, float bottom, float left, float right);

    AlignmentX getAlignmentX() const;
    Container* setAlignmentX(AlignmentX align);
    AlignmentY getAlignmentY() const;
    Container* setAlignmentY(AlignmentY align);

    Container* setRoot(bool isRoot);
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

    std::vector<ChildDefaults> defaultsStack;
};

} // namespace UI

#include "container.tpp"

#endif // CONTAINER_HPP
