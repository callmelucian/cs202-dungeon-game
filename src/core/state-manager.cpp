#include "state-manager.hpp"
#include "game.hpp"

void StateManager::draw() const {
    // get render window & pass them down the GameState object
    Game &game = Game::getInstance();
    sf::RenderWindow &window = game.getWindow();

    // draw current state
    const GameState* curr = currentState();
    if (curr) curr->draw(window);
}

void StateManager::update (float deltaTime) {
    // update current state
    GameState* curr = currentState();
    if (curr) curr->update(deltaTime);
}

void StateManager::handleEvents (sf::Event &event) {
    // pass event to current state for it to handle
    GameState* curr = currentState();
    if (curr) curr->handleEvents(event);
}

void StateManager::pushState (std::unique_ptr<GameState> state) {
    states.push_back(std::move(state));
}

void StateManager::popState() {
    if (states.size()) states.pop_back();
}

void StateManager::changeState (std::unique_ptr<GameState> state) {
    popState(), pushState(std::move(state));
}

GameState* StateManager::currentState() const {
    if (states.empty()) return nullptr;
    return states.back().get();
}