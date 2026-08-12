#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics.hpp>
#include "../global-settings/setting-manager.hpp"
#include "state-manager.hpp"
#include "run-state.hpp"

constexpr int screenWidth = 1800;
constexpr int screenHeight = 900;

class Game {
public:
    static Game& getInstance();

    void runMainLoop();
    sf::RenderWindow& getWindow();
    bool isRunning() const;
    RunState& getRunState();

private:
    sf::ContextSettings contextSettings;
    sf::RenderWindow renderWindow;
    StateManager stateManager;
    bool running;
    RunState runState;

    Game();
    Game (const Game&) = delete;
    Game& operator= (const Game&) = delete;
};

#endif // GAME_HPP