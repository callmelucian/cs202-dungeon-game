#ifndef GAME_STATE
#define GAME_STATE

class StateManager;

class GameState {
public:
    GameState (StateManager& manager);

    virtual void update (float deltaTime) = 0;
    virtual void draw (sf::RenderWindow& window) const = 0;
    virtual void handleEvents (sf::Event& event) = 0;

private:
    StateManager& stateManager;
};

#endif // GAME_STATE