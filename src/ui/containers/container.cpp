#include "container.hpp"
#include <algorithm>

namespace UI {

Container::Container()
    : paddingTop(0.f),
      paddingBottom(0.f),
      paddingLeft(0.f),
      paddingRight(0.f),
      alignmentX(AlignmentX::Left),
      alignmentY(AlignmentY::Top),
      isRootNode(false) {
    fixedWidth = 50.f;
    fixedHeight = 50.f;
}

Container::~Container() {
}

void Container::draw(sf::RenderTarget& target) const {
    for (const auto& child : children) {
        child->draw(target);
    }
}

void Container::handleEvent(const sf::Event& event) {
    for (auto& child : children) {
        child->handleEvent(event);
    }
}

void Container::update(float dt) {
    for (auto& child : children) {
        child->update(dt);
    }
}

void Container::onColorPaletteChanged(const ColorPalette& palette) {
    for (auto& child : children) {
        child->onColorPaletteChanged(palette);
    }
}

void Container::computeSize(sf::Vector2f availableSize) {
    // Pass 1: Resolve own size for Fixed/Expanded
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

    // Compute content area based on current size (if size is known)
    sf::Vector2f contentArea(
        std::max(0.f, size.x - paddingLeft - paddingRight),
        std::max(0.f, size.y - paddingTop - paddingBottom)
    );

    // Recursively resolve children (Pass 1 for children)
    for (auto& child : children) {
        child->computeSize(contentArea);
    }

    // Pass 2: Resolve own size for Contained
    if (modeX == SizeMode::Contained) {
        bool hasExpandedChild = false;
        for (const auto& child : children) {
            if (child->getModeX() == SizeMode::Expanded) {
                hasExpandedChild = true;
                break;
            }
        }
        if (hasExpandedChild || children.empty()) {
            size.x = 0.f;
        }
        else {
            float maxChildExtent = 0.f;
            for (const auto& child : children) {
                float childExtent = child->getSize().x + child->getMarginLeft() + child->getMarginRight();
                if (childExtent > maxChildExtent) {
                    maxChildExtent = childExtent;
                }
            }
            size.x = maxChildExtent + paddingLeft + paddingRight;
        }
    }

    if (modeY == SizeMode::Contained) {
        bool hasExpandedChild = false;
        for (const auto& child : children) {
            if (child->getModeY() == SizeMode::Expanded) {
                hasExpandedChild = true;
                break;
            }
        }
        if (hasExpandedChild || children.empty()) {
            size.y = 0.f;
        }
        else {
            float maxChildExtent = 0.f;
            for (const auto& child : children) {
                float childExtent = child->getSize().y + child->getMarginTop() + child->getMarginBottom();
                if (childExtent > maxChildExtent) {
                    maxChildExtent = childExtent;
                }
            }
            size.y = maxChildExtent + paddingTop + paddingBottom;
        }
    }
}

void Container::setPosition(sf::Vector2f pos) {
    position = pos;

    for (auto& child : children) {
        sf::Vector2f childPos = position;

        // X-axis alignment
        if (modeX != SizeMode::Contained) {
            if (alignmentX == AlignmentX::Left) {
                childPos.x = position.x + paddingLeft + child->getMarginLeft();
            }
            else if (alignmentX == AlignmentX::Center) {
                childPos.x = position.x + (size.x - child->getSize().x) / 2.f;
            }
            else if (alignmentX == AlignmentX::Right) {
                childPos.x = position.x + size.x - child->getSize().x - paddingRight - child->getMarginRight();
            }
        }
        else {
            childPos.x = position.x + paddingLeft + child->getMarginLeft();
        }

        // Y-axis alignment
        if (modeY != SizeMode::Contained) {
            if (alignmentY == AlignmentY::Top) {
                childPos.y = position.y + paddingTop + child->getMarginTop();
            }
            else if (alignmentY == AlignmentY::Middle) {
                childPos.y = position.y + (size.y - child->getSize().y) / 2.f;
            }
            else if (alignmentY == AlignmentY::Bottom) {
                childPos.y = position.y + size.y - child->getSize().y - paddingBottom - child->getMarginBottom();
            }
        }
        else {
            childPos.y = position.y + paddingTop + child->getMarginTop();
        }

        child->setPosition(childPos);
    }
}

Component* Container::addChild(std::unique_ptr<Component> child) {
    if (child) {
        child->setParent(this);
        Component* ptr = child.get();
        children.push_back(std::move(child));
        return ptr;
    }
    return nullptr;
}

const std::vector<std::unique_ptr<Component>>& Container::getChildren() const {
    return children;
}

Container* Container::setChildDefaults(ChildDefaults defaults) {
    defaultsStack.clear();
    defaultsStack.push_back(defaults);
    return this;
}

Container* Container::clearChildDefaults() {
    defaultsStack.clear();
    return this;
}

Container* Container::pushChildDefaults(ChildDefaults defaults) {
    defaultsStack.push_back(defaults);
    return this;
}

Container* Container::popChildDefaults() {
    if (!defaultsStack.empty()) {
        defaultsStack.pop_back();
    }
    return this;
}

std::optional<ChildDefaults> Container::activeChildDefaults() const {
    if (defaultsStack.empty()) {
        return std::nullopt;
    }
    return defaultsStack.back();
}

float Container::getPaddingTop() const { return paddingTop; }
Container* Container::setPaddingTop(float padding) { paddingTop = padding; return this; }

float Container::getPaddingBottom() const { return paddingBottom; }
Container* Container::setPaddingBottom(float padding) { paddingBottom = padding; return this; }

float Container::getPaddingLeft() const { return paddingLeft; }
Container* Container::setPaddingLeft(float padding) { paddingLeft = padding; return this; }

float Container::getPaddingRight() const { return paddingRight; }
Container* Container::setPaddingRight(float padding) { paddingRight = padding; return this; }

Container* Container::setPadding(float top, float bottom, float left, float right) {
    paddingTop = top;
    paddingBottom = bottom;
    paddingLeft = left;
    paddingRight = right;
    return this;
}

AlignmentX Container::getAlignmentX() const { return alignmentX; }
Container* Container::setAlignmentX(AlignmentX align) { alignmentX = align; return this; }

AlignmentY Container::getAlignmentY() const { return alignmentY; }
Container* Container::setAlignmentY(AlignmentY align) { alignmentY = align; return this; }

Container* Container::setRoot(bool isRoot) { isRootNode = isRoot; return this; }
bool Container::isRoot() const { return isRootNode; }

} // namespace UI
