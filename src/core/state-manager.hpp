#ifndef STATE_MANAGER
#define STATE_MANAGER

#include <SFML/Graphics.hpp>
#include <vector>
#include<memory>

#include "game-state.hpp"

class StateManager {
public:
    void pushState (std::unique_ptr<GameState> state);
    void popState();
    void changeState (std::unique_ptr<GameState> state);
    void update (float deltaTime);
    void draw() const;
    void handleEvents (sf::Event &event);

    GameState* currentState() const;

private:
    std::vector<std::unique_ptr<GameState>> states;
};

#endif // STATE_MANAGER