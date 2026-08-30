#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

#include "../ui/containers/container.hpp"
#include "../ui/containers/flex-box.hpp"
#include "../ui/base/component.hpp"
#include <memory>
#include <optional>

class StateManager;

class GameState {
public:
    GameState (StateManager& manager);
    virtual ~GameState();

    virtual void update (float deltaTime);
    virtual void draw (sf::RenderWindow& window) const;
    virtual void handleEvents (sf::Event& event);
    virtual bool isTransparent() const { return false; }

protected:
    StateManager& stateManager;
    std::unique_ptr<UI::Container> root;
    std::optional<sf::Sprite> backgroundSprite;
    bool drawBackground;
};

#endif // GAME_STATE_HPP