#ifndef SETTER_MIXIN_HPP
#define SETTER_MIXIN_HPP

#include "component.hpp"

namespace UI {

enum class AlignmentX;
enum class AlignmentY;

template <typename Derived>
class SetterMixin {
public:
    Derived* setModeX(SizeMode mode);
    Derived* setModeY(SizeMode mode);
    Derived* setFixedWidth(float width);
    Derived* setFixedHeight(float height);
    Derived* setFixedSize(sf::Vector2f size);
    Derived* setMarginTop(float margin);
    Derived* setMarginBottom(float margin);
    Derived* setMarginLeft(float margin);
    Derived* setMarginRight(float margin);
    Derived* setMargins(float top, float bottom, float left, float right);
    Derived* setMargin(float t, float r, float b, float l);

    Derived* setPadding(float t, float r, float b, float l)
        requires requires(Derived* t) { t->getPaddingTop(); };

    Derived* setPaddingTop(float padding)
        requires requires(Derived* t) { t->getPaddingTop(); };

    Derived* setPaddingBottom(float padding)
        requires requires(Derived* t) { t->getPaddingTop(); };

    Derived* setPaddingLeft(float padding)
        requires requires(Derived* t) { t->getPaddingTop(); };

    Derived* setPaddingRight(float padding)
        requires requires(Derived* t) { t->getPaddingTop(); };

    Derived* setAlignmentX(AlignmentX align)
        requires requires(Derived* t) { t->getAlignmentX(); };

    Derived* setAlignmentY(AlignmentY align)
        requires requires(Derived* t) { t->getAlignmentX(); };

    Derived* setRoot(bool isRoot)
        requires requires(Derived* t) { t->isRoot(); };

private:
    Derived* self();
};

} // namespace UI

#endif // SETTER_MIXIN_HPP
