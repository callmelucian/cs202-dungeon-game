#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include "../base/component.hpp"
#include <vector>
#include <memory>

namespace ui {

class Container : public Component {
public:
    Container();

    void addComponent(const std::shared_ptr<Component>& child);
    void clear();
    const std::vector<std::shared_ptr<Component>>& getChildren() const;

    bool handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    void updateColors(const ColorPalette& palette) override;

    virtual void onChildAdded(const std::shared_ptr<Component>& child);
    virtual void onContainerCleared();

    std::vector<std::shared_ptr<Component>> children;
};

} // namespace ui

#endif // CONTAINER_HPP
