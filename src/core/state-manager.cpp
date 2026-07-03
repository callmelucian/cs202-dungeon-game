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
    applyPendingChanges();
    // update current state
    GameState* curr = currentState();
    if (curr) curr->update(deltaTime);
}

void StateManager::handleEvents (sf::Event &event) {
    applyPendingChanges();
    // pass event to current state for it to handle
    GameState* curr = currentState();
    if (curr) curr->handleEvents(event);
}

void StateManager::pushState (std::unique_ptr<GameState> state) {
    pendingActions.push_back({ActionType::Push, std::move(state)});
}

void StateManager::popState() {
    pendingActions.push_back({ActionType::Pop, nullptr});
}

void StateManager::changeState (std::unique_ptr<GameState> state) {
    pendingActions.push_back({ActionType::Change, std::move(state)});
}

void StateManager::clearAndSetState (std::unique_ptr<GameState> state) {
    pendingActions.push_back({ActionType::ClearAndSet, std::move(state)});
}

GameState* StateManager::currentState() const {
    if (states.empty()) return nullptr;
    return states.back().get();
}

void StateManager::applyPendingChanges() {
    while (!pendingActions.empty()) {
        std::vector<PendingAction> actions = std::move(pendingActions);
        pendingActions.clear();

        for (auto& action : actions) {
            switch (action.type) {
                case ActionType::Push:
                    states.push_back(std::move(action.state));
                    break;
                case ActionType::Pop:
                    if (!states.empty()) {
                        states.pop_back();
                    }
                    break;
                case ActionType::Change:
                    if (!states.empty()) {
                        states.pop_back();
                    }
                    states.push_back(std::move(action.state));
                    break;
                case ActionType::ClearAndSet:
                    states.clear();
                    states.push_back(std::move(action.state));
                    break;
            }
        }
    }
}