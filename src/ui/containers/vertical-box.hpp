#ifndef VERTICAL_BOX_HPP
#define VERTICAL_BOX_HPP

#include "container.hpp"

namespace UI {

class VerticalBox : public Container {
public:
    enum class Alignment {
        Left,
        Center,
        Right
    };

    VerticalBox(Alignment align = Alignment::Center, float space = 10.0f, float pad = 10.0f);

    void setPosition(const sf::Vector2f& pos) override;
    void setSize(const sf::Vector2f& sz) override;

    void setAlignment(Alignment align);
    Alignment getAlignment() const;

    void setSpacing(float space);
    float getSpacing() const;

    void setPadding(float pad);
    float getPadding() const;

    void layout();

protected:
    void onChildAdded(const std::shared_ptr<Component>& child) override;
    void onContainerCleared() override;

private:
    Alignment alignment;
    float spacing;
    float padding;
};

} // namespace ui

#endif // VERTICAL_BOX_HPP
