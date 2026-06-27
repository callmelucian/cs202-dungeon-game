#include "component.hpp"

namespace UI {

Component::Component()
    : parent(nullptr),
      position(0.f, 0.f),
      size(0.f, 0.f),
      modeX(SizeMode::Fixed),
      modeY(SizeMode::Fixed),
      fixedWidth(0.f),
      fixedHeight(0.f),
      marginTop(0.f),
      marginBottom(0.f),
      marginLeft(0.f),
      marginRight(0.f) {
    ColorPaletteManager::getInstance().addObserver(this);
}

Component::~Component() {
    ColorPaletteManager::getInstance().removeObserver(this);
}

Component* Component::getParent() const {
    return parent;
}

void Component::setParent(Component* newParent) {
    parent = newParent;
}

sf::Vector2f Component::getPosition() const {
    return position;
}

sf::Vector2f Component::getSize() const {
    return size;
}

SizeMode Component::getModeX() const {
    return modeX;
}

void Component::setModeX(SizeMode mode) {
    modeX = mode;
}

SizeMode Component::getModeY() const {
    return modeY;
}

void Component::setModeY(SizeMode mode) {
    modeY = mode;
}

float Component::getFixedWidth() const {
    return fixedWidth;
}

void Component::setFixedWidth(float width) {
    fixedWidth = width;
}

float Component::getFixedHeight() const {
    return fixedHeight;
}

void Component::setFixedHeight(float height) {
    fixedHeight = height;
}

void Component::setFixedSize(sf::Vector2f sz) {
    fixedWidth = sz.x;
    fixedHeight = sz.y;
}

float Component::getMarginTop() const {
    return marginTop;
}

void Component::setMarginTop(float margin) {
    marginTop = margin;
}

float Component::getMarginBottom() const {
    return marginBottom;
}

void Component::setMarginBottom(float margin) {
    marginBottom = margin;
}

float Component::getMarginLeft() const {
    return marginLeft;
}

void Component::setMarginLeft(float margin) {
    marginLeft = margin;
}

float Component::getMarginRight() const {
    return marginRight;
}

void Component::setMarginRight(float margin) {
    marginRight = margin;
}

void Component::setMargins(float top, float bottom, float left, float right) {
    marginTop = top;
    marginBottom = bottom;
    marginLeft = left;
    marginRight = right;
}

} // namespace UI
