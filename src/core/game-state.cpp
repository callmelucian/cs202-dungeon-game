#include "state-manager.hpp"
#include "game-state.hpp"
#include "../global-settings/setting-manager.hpp"

GameState::GameState (StateManager& manager) : stateManager(manager) {
    // get singleton instances
    SettingManager &settings = SettingManager::getInstance();

    // set root container configuration
    root.setModeX(UI::SizeMode::Fixed);
    root.setFixedWidth(settings.getWindowWidth());
    root.setModeY(UI::SizeMode::Fixed);
    root.setFixedHeight(settings.getWindowHeight());
}

void GameState::update (float deltaTime) {
    SettingManager &settings = SettingManager::getInstance();
    root.computeSize(sf::Vector2f(settings.getWindowWidth(), settings.getWindowHeight()));
    root.setPosition(sf::Vector2f(0.f, 0.f));
    root.update(deltaTime);
}

void GameState::draw (sf::RenderWindow &window) const {
    root.draw(window);
}

void GameState::handleEvents (sf::Event &event) {
    root.handleEvent(event);
}