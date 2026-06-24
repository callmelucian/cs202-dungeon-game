#ifndef GAME
#define GAME

#include <SFML/Graphics.hpp>
#include "../global-settings/setting-manager.hpp"
#include "state-manager.hpp"

const int screenWidth = 1800;
const int screenHeight = 900;

class Game {
public:
    static Game& getInstance();

    void runMainLoop();
    sf::RenderWindow& getWindow() const;
    bool isRunning() const;

private:
    sf::ContextSettings contextSettings;
    sf::RenderWindow renderWindow;
    StateManager stateManager;
    bool running;

    Game();
    Game (const Game&) = delete;
    Game& operator= (const Game&) = delete;
};

#endif // GAME