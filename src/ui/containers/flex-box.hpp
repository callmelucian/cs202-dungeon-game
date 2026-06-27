#ifndef FLEX_BOX_HPP
#define FLEX_BOX_HPP

#include "container.hpp"

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
class FlexBox : public Container {
public:
    FlexBox();
    virtual ~FlexBox() = default;

    void draw(sf::RenderTarget& target) const override;
    void computeSize(sf::Vector2f availableSize) override;
    void setPosition(sf::Vector2f pos) override;

    float getSpacing() const;
    void setSpacing(float gap);

    Distribution getDistribution() const;
    void setDistribution(Distribution dist);

    Overflow getOverflow() const;
    void setOverflow(Overflow flow);

private:
    float spacing;
    Distribution distribution;
    Overflow overflow;
};

using HorizontalBox = FlexBox<Axis::Horizontal>;
using VerticalBox   = FlexBox<Axis::Vertical>;

} // namespace UI

#endif // FLEX_BOX_HPP
