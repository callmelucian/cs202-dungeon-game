#include "game.hpp"
#include "../global-settings/color-palette-manager.hpp"
#include "../global-settings/asset-manager.hpp"
#include "../global-settings/sound-manager.hpp"
#include <filesystem>

#include "states/main-menu-state.hpp"

Game::Game() : running(false) {
    // setup context settings
    contextSettings.antiAliasingLevel = 8;

    // get singleton instances
    SettingManager &settings = SettingManager::getInstance();
    AssetManager &assets = AssetManager::getInstance();

    // helper to resolve path
    auto resolvePath = [](const std::string& path) {
        if (std::filesystem::exists(path)) return path;
        if (std::filesystem::exists("../" + path)) return "../" + path;
        return path;
    };

    // get global settings
    settings.loadSettings(resolvePath("settings.json"));
    unsigned width = settings.getWindowWidth();
    unsigned height = settings.getWindowHeight();

    // load assets via assets manager
    assets.loadFont("regular", resolvePath("assets/typeface/GoogleSansCode-Regular.ttf"));
    assets.loadFont("italic", resolvePath("assets/typeface/GoogleSansCode-Italic.ttf"));
    assets.loadFont("bold", resolvePath("assets/typeface/GoogleSansCode-Bold.ttf"));
    assets.loadFont("bold-italic", resolvePath("assets/typeface/GoogleSansCode-BoldItalic.ttf"));

    // load audio via sound manager
    SoundManager &sounds = SoundManager::getInstance();
    sounds.loadSound("menu-nav", resolvePath("assets/sounds/menu-nav.wav"));
    sounds.loadSound("menu-select", resolvePath("assets/sounds/menu-select.wav"));
    sounds.playMusic(resolvePath("assets/music/fallen-down-reprise.ogg"), true);

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