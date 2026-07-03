#include "SetterMixin.hpp"
#include "text.hpp"
#include "../widgets/button.hpp"
#include "../widgets/slider.hpp"
#include "../widgets/text-input.hpp"
#include "../containers/flex-box.hpp"
#include "../containers/container.hpp"

namespace UI {

template <typename Derived>
Derived* SetterMixin<Derived>::self() {
    return static_cast<Derived*>(this);
}

template <typename Derived>
Derived* SetterMixin<Derived>::setModeX(SizeMode mode) {
    static_cast<Component*>(self())->setModeX(mode);
    return self();
}

template <typename Derived>
Derived* SetterMixin<Derived>::setModeY(SizeMode mode) {
    static_cast<Component*>(self())->setModeY(mode);
    return self();
}

template <typename Derived>
Derived* SetterMixin<Derived>::setFixedWidth(float width) {
    static_cast<Component*>(self())->setFixedWidth(width);
    return self();
}

template <typename Derived>
Derived* SetterMixin<Derived>::setFixedHeight(float height) {
    static_cast<Component*>(self())->setFixedHeight(height);
    return self();
}

template <typename Derived>
Derived* SetterMixin<Derived>::setFixedSize(sf::Vector2f size) {
    static_cast<Component*>(self())->setFixedSize(size);
    return self();
}

template <typename Derived>
Derived* SetterMixin<Derived>::setMarginTop(float margin) {
    static_cast<Component*>(self())->setMarginTop(margin);
    return self();
}

template <typename Derived>
Derived* SetterMixin<Derived>::setMarginBottom(float margin) {
    static_cast<Component*>(self())->setMarginBottom(margin);
    return self();
}

template <typename Derived>
Derived* SetterMixin<Derived>::setMarginLeft(float margin) {
    static_cast<Component*>(self())->setMarginLeft(margin);
    return self();
}

template <typename Derived>
Derived* SetterMixin<Derived>::setMarginRight(float margin) {
    static_cast<Component*>(self())->setMarginRight(margin);
    return self();
}

template <typename Derived>
Derived* SetterMixin<Derived>::setMargins(float top, float bottom, float left, float right) {
    static_cast<Component*>(self())->setMargins(top, bottom, left, right);
    return self();
}

template <typename Derived>
Derived* SetterMixin<Derived>::setMargin(float t, float r, float b, float l) {
    static_cast<Component*>(self())->setMarginTop(t);
    static_cast<Component*>(self())->setMarginRight(r);
    static_cast<Component*>(self())->setMarginBottom(b);
    static_cast<Component*>(self())->setMarginLeft(l);
    return self();
}

template <typename Derived>
Derived* SetterMixin<Derived>::setPadding(float t, float r, float b, float l)
requires requires(Derived* t_ptr) { t_ptr->getPaddingTop(); } {
    self()->setPaddingTop(t);
    self()->setPaddingRight(r);
    self()->setPaddingBottom(b);
    self()->setPaddingLeft(l);
    return self();
}

template <typename Derived>
Derived* SetterMixin<Derived>::setPaddingTop(float padding)
requires requires(Derived* t_ptr) { t_ptr->getPaddingTop(); } {
    static_cast<Container*>(self())->setPaddingTop(padding);
    return self();
}

template <typename Derived>
Derived* SetterMixin<Derived>::setPaddingBottom(float padding)
requires requires(Derived* t_ptr) { t_ptr->getPaddingTop(); } {
    static_cast<Container*>(self())->setPaddingBottom(padding);
    return self();
}

template <typename Derived>
Derived* SetterMixin<Derived>::setPaddingLeft(float padding)
requires requires(Derived* t_ptr) { t_ptr->getPaddingTop(); } {
    static_cast<Container*>(self())->setPaddingLeft(padding);
    return self();
}

template <typename Derived>
Derived* SetterMixin<Derived>::setPaddingRight(float padding)
requires requires(Derived* t_ptr) { t_ptr->getPaddingTop(); } {
    static_cast<Container*>(self())->setPaddingRight(padding);
    return self();
}

template <typename Derived>
Derived* SetterMixin<Derived>::setAlignmentX(AlignmentX align)
requires requires(Derived* t_ptr) { t_ptr->getAlignmentX(); } {
    static_cast<Container*>(self())->setAlignmentX(align);
    return self();
}

template <typename Derived>
Derived* SetterMixin<Derived>::setAlignmentY(AlignmentY align)
requires requires(Derived* t_ptr) { t_ptr->getAlignmentX(); } {
    static_cast<Container*>(self())->setAlignmentY(align);
    return self();
}

template <typename Derived>
Derived* SetterMixin<Derived>::setRoot(bool isRoot)
requires requires(Derived* t_ptr) { t_ptr->isRoot(); } {
    static_cast<Container*>(self())->setRoot(isRoot);
    return self();
}

// Explicit template instantiations
template class SetterMixin<Text>;
template class SetterMixin<Button>;
template class SetterMixin<Slider>;
template class SetterMixin<TextInput>;
template class SetterMixin<HorizontalBox>;
template class SetterMixin<VerticalBox>;

} // namespace UI
