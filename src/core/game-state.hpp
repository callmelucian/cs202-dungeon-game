#ifndef GAME_STATE
#define GAME_STATE

#include "../ui/containers/container.hpp"
#include "../ui/containers/flex-box.hpp"
#include "../ui/base/component.hpp"
#include <memory>

class StateManager;

class GameState {
public:
    GameState (StateManager& manager);
    virtual ~GameState();

    virtual void update (float deltaTime);
    virtual void draw (sf::RenderWindow& window) const;
    virtual void handleEvents (sf::Event& event);

protected:
    StateManager& stateManager;
    std::unique_ptr<UI::Container> root;
};

#endif // GAME_STATE