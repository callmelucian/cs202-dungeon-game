#ifndef FLEX_BOX_HPP
#define FLEX_BOX_HPP

#include "container.hpp"
#include "../base/SetterMixin.hpp"

namespace UI {

enum class Axis {
    Horizontal,
    Vertical
};

enum class Distribution {
    FlexStart,
    FlexEnd,
    SpaceBetween,
    SpaceAround,
    SpaceEvenly
};

enum class Overflow {
    Visible,
    Hidden
};

// Template container representing horizontal or vertical flex layout boxes
template <Axis MainAxis>
class FlexBox : public Container, public SetterMixin<FlexBox<MainAxis>> {
public:
    using SetterMixin<FlexBox<MainAxis>>::setModeX;
    using SetterMixin<FlexBox<MainAxis>>::setModeY;
    using SetterMixin<FlexBox<MainAxis>>::setFixedWidth;
    using SetterMixin<FlexBox<MainAxis>>::setFixedHeight;
    using SetterMixin<FlexBox<MainAxis>>::setFixedSize;
    using SetterMixin<FlexBox<MainAxis>>::setMarginTop;
    using SetterMixin<FlexBox<MainAxis>>::setMarginBottom;
    using SetterMixin<FlexBox<MainAxis>>::setMarginLeft;
    using SetterMixin<FlexBox<MainAxis>>::setMarginRight;
    using SetterMixin<FlexBox<MainAxis>>::setMargins;
    using SetterMixin<FlexBox<MainAxis>>::setMargin;
    using SetterMixin<FlexBox<MainAxis>>::setPadding;
    using SetterMixin<FlexBox<MainAxis>>::setPaddingTop;
    using SetterMixin<FlexBox<MainAxis>>::setPaddingBottom;
    using SetterMixin<FlexBox<MainAxis>>::setPaddingLeft;
    using SetterMixin<FlexBox<MainAxis>>::setPaddingRight;
    using SetterMixin<FlexBox<MainAxis>>::setAlignmentX;
    using SetterMixin<FlexBox<MainAxis>>::setAlignmentY;
    using SetterMixin<FlexBox<MainAxis>>::setRoot;
    using SetterMixin<FlexBox<MainAxis>>::setColor;

    FlexBox();
    virtual ~FlexBox() = default;

    void draw(sf::RenderTarget& target) const override;
    void computeSize(sf::Vector2f availableSize) override;
    void setPosition(sf::Vector2f pos) override;

    float getSpacing() const;
    FlexBox<MainAxis>* setSpacing(float gap);

    Distribution getDistribution() const;
    FlexBox<MainAxis>* setDistribution(Distribution dist);

    Overflow getOverflow() const;
    FlexBox<MainAxis>* setOverflow(Overflow flow);

private:
    float spacing;
    Distribution distribution;
    Overflow overflow;
};

using HorizontalBox = FlexBox<Axis::Horizontal>;
using VerticalBox   = FlexBox<Axis::Vertical>;

} // namespace UI

#endif // FLEX_BOX_HPP
