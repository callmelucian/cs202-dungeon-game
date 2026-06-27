#ifndef GAME_STATE
#define GAME_STATE

#include "../ui/containers/container.hpp"
#include "../ui/containers/flex-box.hpp"
#include "../ui/base/component.hpp"

class StateManager;

class GameState {
public:
    GameState (StateManager& manager);

    void update (float deltaTime);
    void draw (sf::RenderWindow& window) const;
    void handleEvents (sf::Event& event);

private:
    StateManager& stateManager;

protected:
    UI::Container root;
};

#endif // GAME_STATE