#include "flex-box.hpp"
#include <vector>
#include <algorithm>
#include <cmath>

namespace UI {

template <Axis MainAxis>
FlexBox<MainAxis>::FlexBox()
    : Container(),
      spacing(0.f),
      distribution(Distribution::FlexStart),
      overflow(Overflow::Visible) {
}

template <Axis MainAxis>
float FlexBox<MainAxis>::getSpacing() const {
    return spacing;
}

template <Axis MainAxis>
FlexBox<MainAxis>* FlexBox<MainAxis>::setSpacing(float gap) {
    spacing = gap;
    return this;
}

template <Axis MainAxis>
Distribution FlexBox<MainAxis>::getDistribution() const {
    return distribution;
}

template <Axis MainAxis>
FlexBox<MainAxis>* FlexBox<MainAxis>::setDistribution(Distribution dist) {
    distribution = dist;
    return this;
}

template <Axis MainAxis>
Overflow FlexBox<MainAxis>::getOverflow() const {
    return overflow;
}

template <Axis MainAxis>
FlexBox<MainAxis>* FlexBox<MainAxis>::setOverflow(Overflow flow) {
    overflow = flow;
    return this;
}

template <Axis MainAxis>
void FlexBox<MainAxis>::draw(sf::RenderTarget& target) const {
    if (overflow == Overflow::Hidden) {
        sf::View originalView = target.getView();
        sf::Vector2u targetSize = target.getSize();
        
        if (targetSize.x > 0 && targetSize.y > 0) {
            sf::Vector2i topLeftPix = target.mapCoordsToPixel(position, originalView);
            sf::Vector2i bottomRightPix = target.mapCoordsToPixel(position + size, originalView);
            
            float normLeft = static_cast<float>(topLeftPix.x) / targetSize.x;
            float normTop = static_cast<float>(topLeftPix.y) / targetSize.y;
            float normWidth = static_cast<float>(bottomRightPix.x - topLeftPix.x) / targetSize.x;
            float normHeight = static_cast<float>(bottomRightPix.y - topLeftPix.y) / targetSize.y;
            
            sf::FloatRect viewportRect(
                {std::max(0.f, std::min(1.f, normLeft)), std::max(0.f, std::min(1.f, normTop))},
                {std::max(0.f, std::min(1.f - normLeft, normWidth)), std::max(0.f, std::min(1.f - normTop, normHeight))}
            );
            
            sf::View clipView(position + size / 2.f, size);
            clipView.setViewport(viewportRect);
            
            target.setView(clipView);
            Container::draw(target);
            target.setView(originalView);
        }
        else {
            Container::draw(target);
        }
    }
    else {
        Container::draw(target);
    }
}

template <Axis MainAxis>
void FlexBox<MainAxis>::computeSize(sf::Vector2f availableSize) {
    if (modeX == SizeMode::Fixed) {
        size.x = fixedWidth;
    }
    else if (modeX == SizeMode::Expanded) {
        if (!isRootNode) {
            size.x = availableSize.x - marginLeft - marginRight;
        }
        else {
            size.x = availableSize.x;
        }
    }

    if (modeY == SizeMode::Fixed) {
        size.y = fixedHeight;
    }
    else if (modeY == SizeMode::Expanded) {
        if (!isRootNode) {
            size.y = availableSize.y - marginTop - marginBottom;
        }
        else {
            size.y = availableSize.y;
        }
    }

    constexpr bool isHoriz = (MainAxis == Axis::Horizontal);
    
    float mainPaddingStart = isHoriz ? paddingLeft : paddingTop;
    float mainPaddingEnd   = isHoriz ? paddingRight : paddingBottom;
    float crossPaddingStart = isHoriz ? paddingTop : paddingLeft;
    float crossPaddingEnd   = isHoriz ? paddingBottom : paddingRight;

    float mainSizeVal = isHoriz ? size.x : size.y;
    float crossSizeVal = isHoriz ? size.y : size.x;

    bool isMainContained = isHoriz ? (modeX == SizeMode::Contained) : (modeY == SizeMode::Contained);
    bool isCrossContained = isHoriz ? (modeY == SizeMode::Contained) : (modeX == SizeMode::Contained);

    float mainContentLength = std::max(0.f, mainSizeVal - mainPaddingStart - mainPaddingEnd);
    float crossContentLength = std::max(0.f, crossSizeVal - crossPaddingStart - crossPaddingEnd);

    size_t n = children.size();
    int k = 0;
    float fixedMainTotal = 0.f;

    for (const auto& child : children) {
        bool isChildExpandedMain = isHoriz ? (child->getModeX() == SizeMode::Expanded) 
                                           : (child->getModeY() == SizeMode::Expanded);
        if (isChildExpandedMain) {
            k++;
        }
        else {
            sf::Vector2f childConstraint;
            if (isHoriz) {
                childConstraint = sf::Vector2f(mainContentLength, crossContentLength);
            }
            else {
                childConstraint = sf::Vector2f(crossContentLength, mainContentLength);
            }
            child->computeSize(childConstraint);

            float childMainMargin = isHoriz ? (child->getMarginLeft() + child->getMarginRight())
                                            : (child->getMarginTop() + child->getMarginBottom());
            float childMainSize = isHoriz ? child->getSize().x : child->getSize().y;
            fixedMainTotal += childMainSize + childMainMargin;
        }
    }

    float spacingTotal = (n > 1) ? (static_cast<float>(n - 1) * spacing) : 0.f;

    if (isMainContained) {
        if (k > 0 || n == 0) {
            mainSizeVal = 0.f;
        }
        else {
            mainSizeVal = fixedMainTotal + spacingTotal + mainPaddingStart + mainPaddingEnd;
        }
        mainContentLength = std::max(0.f, mainSizeVal - mainPaddingStart - mainPaddingEnd);
        if (isHoriz) size.x = mainSizeVal; else size.y = mainSizeVal;
    }

    if (k > 0) {
        float freeSpace = mainContentLength - fixedMainTotal - spacingTotal;
        float totalExpandedMargins = 0.f;
        for (const auto& child : children) {
            bool isChildExpandedMain = isHoriz ? (child->getModeX() == SizeMode::Expanded) 
                                               : (child->getModeY() == SizeMode::Expanded);
            if (isChildExpandedMain) {
                totalExpandedMargins += isHoriz ? (child->getMarginLeft() + child->getMarginRight())
                                                : (child->getMarginTop() + child->getMarginBottom());
            }
        }
        float m_expanded = (freeSpace - totalExpandedMargins) / static_cast<float>(k);
        if (m_expanded < 0.f) m_expanded = 0.f;

        for (auto& child : children) {
            bool isChildExpandedMain = isHoriz ? (child->getModeX() == SizeMode::Expanded) 
                                               : (child->getModeY() == SizeMode::Expanded);
            if (isChildExpandedMain) {
                sf::Vector2f childConstraint;
                if (isHoriz) {
                    childConstraint = sf::Vector2f(m_expanded, crossContentLength);
                }
                else {
                    childConstraint = sf::Vector2f(crossContentLength, m_expanded);
                }
                child->computeSize(childConstraint);
            }
        }
    }

    if (isCrossContained) {
        bool hasExpandedChildCross = false;
        for (const auto& child : children) {
            bool isChildExpandedCross = isHoriz ? (child->getModeY() == SizeMode::Expanded)
                                                : (child->getModeX() == SizeMode::Expanded);
            if (isChildExpandedCross) {
                hasExpandedChildCross = true;
                break;
            }
        }
        if (hasExpandedChildCross || n == 0) {
            crossSizeVal = 0.f;
        }
        else {
            float maxChildCrossExtent = 0.f;
            for (const auto& child : children) {
                float childCrossSize = isHoriz ? child->getSize().y : child->getSize().x;
                float childCrossMargin = isHoriz ? (child->getMarginTop() + child->getMarginBottom())
                                                 : (child->getMarginLeft() + child->getMarginRight());
                if (childCrossSize + childCrossMargin > maxChildCrossExtent) {
                    maxChildCrossExtent = childCrossSize + childCrossMargin;
                }
            }
            crossSizeVal = maxChildCrossExtent + crossPaddingStart + crossPaddingEnd;
        }
        if (isHoriz) size.y = crossSizeVal; else size.x = crossSizeVal;
    }
}

template <Axis MainAxis>
void FlexBox<MainAxis>::setPosition(sf::Vector2f pos) {
    position = pos;
    size_t n = children.size();
    if (n == 0) return;

    constexpr bool isHoriz = (MainAxis == Axis::Horizontal);

    float mainPaddingStart = isHoriz ? paddingLeft : paddingTop;
    float crossPaddingStart = isHoriz ? paddingTop : paddingLeft;

    float mainSizeVal = isHoriz ? size.x : size.y;
    float crossSizeVal = isHoriz ? size.y : size.x;

    float mainContentLength = std::max(0.f, mainSizeVal - mainPaddingStart - (isHoriz ? paddingRight : paddingBottom));
    float crossContentLength = std::max(0.f, crossSizeVal - crossPaddingStart - (isHoriz ? paddingBottom : paddingRight));

    int k = 0;
    float fixedMainTotal = 0.f;
    for (const auto& child : children) {
        bool isChildExpandedMain = isHoriz ? (child->getModeX() == SizeMode::Expanded) 
                                           : (child->getModeY() == SizeMode::Expanded);
        if (isChildExpandedMain) {
            k++;
        }
        else {
            float childMainMargin = isHoriz ? (child->getMarginLeft() + child->getMarginRight())
                                            : (child->getMarginTop() + child->getMarginBottom());
            float childMainSize = isHoriz ? child->getSize().x : child->getSize().y;
            fixedMainTotal += childMainSize + childMainMargin;
        }
    }

    std::vector<float> gaps(n + 1, 0.f);

    if (k > 0) {
        gaps[0] = 0.f;
        for (size_t i = 1; i < n; i++) {
            gaps[i] = spacing;
        }
        gaps[n] = 0.f;
    }
    else {
        float S_total = fixedMainTotal;
        float G = mainContentLength - S_total;
        if (G < 0.f) G = 0.f;

        if (distribution == Distribution::FlexStart) {
            gaps[0] = 0.f;
            for (size_t i = 1; i < n; i++) gaps[i] = 0.f;
            gaps[n] = G;
        }
        else if (distribution == Distribution::FlexEnd) {
            gaps[0] = G;
            for (size_t i = 1; i < n; i++) gaps[i] = 0.f;
            gaps[n] = 0.f;
        }
        else if (distribution == Distribution::SpaceBetween) {
            if (n == 1) {
                gaps[0] = 0.f;
                gaps[1] = G;
            }
            else {
                gaps[0] = 0.f;
                float u = G / static_cast<float>(n - 1);
                for (size_t i = 1; i < n; i++) gaps[i] = u;
                gaps[n] = 0.f;
            }
        }
        else if (distribution == Distribution::SpaceAround) {
            float u = G / static_cast<float>(n);
            gaps[0] = u / 2.f;
            for (size_t i = 1; i < n; i++) gaps[i] = u;
            gaps[n] = u / 2.f;
        }
        else if (distribution == Distribution::SpaceEvenly) {
            float u = G / static_cast<float>(n + 1);
            for (size_t i = 0; i <= n; i++) gaps[i] = u;
        }
    }

    float originMain = isHoriz ? (position.x + paddingLeft) : (position.y + paddingTop);
    float originCross = isHoriz ? (position.y + paddingTop) : (position.x + paddingLeft);
    float cursor = gaps[0];

    for (size_t i = 0; i < n; i++) {
        auto& child = children[i];
        float childMarginStartMain = isHoriz ? child->getMarginLeft() : child->getMarginTop();
        float childMarginEndMain   = isHoriz ? child->getMarginRight() : child->getMarginBottom();
        float childSizeMain        = isHoriz ? child->getSize().x : child->getSize().y;

        float mainPos = originMain + cursor + childMarginStartMain;
        
        float childSizeCross = isHoriz ? child->getSize().y : child->getSize().x;
        float childMarginStartCross = isHoriz ? child->getMarginTop() : child->getMarginLeft();
        float childMarginEndCross = isHoriz ? child->getMarginBottom() : child->getMarginRight();
        float childExtentCross = childSizeCross + childMarginStartCross + childMarginEndCross;

        float crossOffset = 0.f;
        
        if (isHoriz) {
            if (alignmentY == AlignmentY::Top) {
                crossOffset = childMarginStartCross;
            }
            else if (alignmentY == AlignmentY::Middle) {
                crossOffset = (crossContentLength - childExtentCross) / 2.f + childMarginStartCross;
            }
            else if (alignmentY == AlignmentY::Bottom) {
                crossOffset = crossContentLength - childExtentCross + childMarginStartCross;
            }
        }
        else {
            if (alignmentX == AlignmentX::Left) {
                crossOffset = childMarginStartCross;
            }
            else if (alignmentX == AlignmentX::Center) {
                crossOffset = (crossContentLength - childExtentCross) / 2.f + childMarginStartCross;
            }
            else if (alignmentX == AlignmentX::Right) {
                crossOffset = crossContentLength - childExtentCross + childMarginStartCross;
            }
        }

        float crossPos = originCross + crossOffset;

        sf::Vector2f finalPos;
        if (isHoriz) {
            finalPos = sf::Vector2f(mainPos, crossPos);
        }
        else {
            finalPos = sf::Vector2f(crossPos, mainPos);
        }

        child->setPosition(finalPos);

        cursor += childMarginStartMain + childSizeMain + childMarginEndMain + gaps[i + 1];
    }
}

// Explicit instantiations to compile template codes in translation unit
template class FlexBox<Axis::Horizontal>;
template class FlexBox<Axis::Vertical>;

} // namespace UI
