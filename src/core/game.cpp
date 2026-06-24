#include "game.hpp"
#include "../global-settings/color-palette-manager.hpp"
#include "states/main-menu-state.hpp"

Game::Game() : running(false) {
    // setup context settings
    contextSettings.antiAliasingLevel = 8;

    // get global settings
    SettingManager &settings = SettingManager::getInstance();
    settings.loadSettings("settings.json");
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

sf::RenderWindow& Game::getWindow() const {
    return const_cast<sf::RenderWindow&>(renderWindow);
}

void Game::runMainLoop() {
    running = true;

    // Push initial main menu state
    stateManager.pushState(std::make_unique<MainMenuState>(stateManager));

    sf::Clock clock;

    while (renderWindow.isOpen()) {
        // event listener
        while (std::optional<sf::Event> event = renderWindow.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                renderWindow.close();
            } else {
                stateManager.handleEvents(*event);
            }
        }

        float dt = clock.restart().asSeconds();
        stateManager.update(dt);

        sf::Color bgColor = ColorPaletteManager::getInstance().getPalette().background;
        renderWindow.clear(bgColor);
        stateManager.draw();
        renderWindow.display();
    }
    SettingManager::getInstance().saveSettings("settings.json");
    running = false;
}