#include "state-manager.hpp"
#include "game.hpp"

void StateManager::draw() {
    // get render window & pass them down the GameState object
    Game &game = Game::getInstance();
    sf::RenderWindow &window = game.getWindow();
}