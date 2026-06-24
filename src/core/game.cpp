#include "game.hpp"

Game::Game() : running(false) {
    // setup context settings
    contextSettings.antiAliasingLevel = 8;

    // get global settings
    SettingManager &settings = SettingManager::getInstance();
    unsigned width = settings.getWindowWidth();
    unsigned height = settings.getWindowHeight();

    // setup render window
    renderWindow.create(
        sf::VideoMode({width, height}),
        "Echoes of the Ashen Vault",
        sf::Style::Default,
        sf::State::Windowed,
        contextSettings
    );
    renderWindow.setPosition({50, 50});
    renderWindow.setFramerateLimit(60);
}

Game& Game::getInstance() {
    static Game instance;
    return instance;
}

bool Game::isRunning() const {
    return running && renderWindow.isOpen();
}

void Game::runMainLoop() {
    running = true;
    while (renderWindow.isOpen()) {
        // event listener
        while (const std::optional<sf::Event> event = renderWindow.pollEvent()) {
            if (event->is<sf::Event::Closed>()) renderWindow.close();
            // ask state manager to handle current event
        }
    }
    running = false;
}