#ifndef GAME
#define GAME

#include <SFML/Graphics.hpp>
#include "../global-settings/setting-manager.hpp"

const int screenWidth = 1800;
const int screenHeight = 900;

class Game {
public:
    static Game& getInstance() {
        static Game instance;
        return instance;
    }

    void runMainLoop();
    sf::RenderWindow& getWindow() const;
    bool isRunning() const;

private:
    sf::ContextSettings contextSettings;
    sf::RenderWindow renderWindow;
    bool running;
    // StateManager stateManager;

    Game();
    Game (const Game&) = delete;
    Game& operator= (const Game&) = delete;
};

#endif // GAME