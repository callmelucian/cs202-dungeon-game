#ifndef GAME_STATE
#define GAME_STATE

#include "state-manager.hpp"

class GameState {
public:
    GameState (StateManager& manager);

    virtual void update (float deltaTime);
    virtual void draw (sf::RenderWindow& window);
    virtual void handleEvents (sf::Event& event);

private:
    StateManager& stateManager;
};

#endif // GAME_STATE