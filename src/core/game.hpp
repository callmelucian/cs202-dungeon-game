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

    void setActiveWorldView(const sf::View& view) { activeWorldView = view; }
    const sf::View& getActiveWorldView() const { return activeWorldView; }

private:
    sf::ContextSettings contextSettings;
    sf::RenderWindow renderWindow;
    sf::View activeWorldView;
    StateManager stateManager;
    bool running;
    RunState runState;

    Game();
    Game (const Game&) = delete;
    Game& operator= (const Game&) = delete;
};

#endif // GAME_HPP