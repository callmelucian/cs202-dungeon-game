#include "game.hpp"
#include "../global-settings/color-palette-manager.hpp"
#include "../global-settings/asset-manager.hpp"
#include "../global-settings/sound-manager.hpp"
#include "../global-settings/animation-manager.hpp"
#include "../global-settings/tile-manager.hpp"
#include <filesystem>
#include <cstdlib>
#include <ctime>

#include "states/main-menu-state.hpp"

Game::Game() : running(false) {
    // Seed RNG for Item scatter and WaveSpawner spawn positions
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

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
    assets.loadFont("header", resolvePath("assets/typeface/press-start/PressStart2P-Regular.ttf"));
    assets.loadFont("regular", resolvePath("assets/typeface/cascadia/CascadiaMono-Regular.ttf"));
    // assets.loadFont("medium", resolvePath("assets/typeface/pixelify-sans/PixelifySans-Medium.ttf"));
    // assets.loadFont("semibold", resolvePath("assets/typeface/pixelify-sans/PixelifySans-SemiBold.ttf"));
    assets.loadFont("bold", resolvePath("assets/typeface/cascadia/CascadiaMono-Bold.ttf"));
    assets.loadFont("italic", resolvePath("assets/typeface/cascadia/CascadiaMono-Italic.ttf"));
    assets.loadFont("bold-italic", resolvePath("assets/typeface/cascadia/CascadiaMono-BoldItalic.ttf"));
    // assets.loadTexture("character-sprite-sheet", resolvePath("assets/textures/character-sprite-sheet.png"));
    assets.loadTexture("wraithblade-final", resolvePath("assets/textures/wraithblade-final.png"));
    assets.loadTexture("voidcaster-final", resolvePath("assets/textures/voidcaster-final.png"));
    assets.loadTexture("iron-shell", resolvePath("assets/textures/iron-shell.png"));
    assets.loadTexture("main-bg", resolvePath("assets/backgrounds/main-background.png"));
    
    // Load enemy textures
    assets.loadTexture("waterlogged-scribe", resolvePath("assets/textures/waterlogged-scribe.png"));
    assets.loadTexture("shard-soldier", resolvePath("assets/textures/shard-soldier.png"));
    assets.loadTexture("bone-sprinter", resolvePath("assets/textures/bone-sprinter.png"));
    assets.loadTexture("boss-malachar-echo-staff", resolvePath("assets/textures/boss-malachar-echo-staff.png"));
    assets.loadTexture("boss-malachar-sword", resolvePath("assets/textures/boss-malachar-sword.png"));
    
    // Load HUD & Projectile textures
    assets.loadTexture("health-bar", resolvePath("assets/textures/health-bar.png"));
    assets.loadTexture("arrow", resolvePath("assets/textures/arrow.png"));
    assets.loadTexture("items", resolvePath("assets/textures/items.png"));

    // load audio via sound manager
    SoundManager &sounds = SoundManager::getInstance();
    sounds.loadSound("menu-nav", resolvePath("assets/sounds/menu-nav.wav"));
    sounds.loadSound("menu-select", resolvePath("assets/sounds/menu-select.wav"));
    sounds.loadSound("switch", resolvePath("assets/sounds/switch.wav"));
    sounds.loadSound("swing", resolvePath("assets/sounds/swing.wav"));
    sounds.loadSound("shoot", resolvePath("assets/sounds/shoot.wav"));
    sounds.loadSound("enemy-hit", resolvePath("assets/sounds/enemy-hit.wav"));
    sounds.loadSound("pickup", resolvePath("assets/sounds/pickup.wav"));
    sounds.loadSound("echo-collect", resolvePath("assets/sounds/echo-collect.wav"));
    sounds.loadSound("boss-phase", resolvePath("assets/sounds/boss-phase.wav"));
    sounds.playMusic(resolvePath("assets/music/fallen-down-reprise.ogg"), true);

    // load animations and tiles
    AnimationManager &animations = AnimationManager::getInstance();
    animations.loadAllAnimations(resolvePath("assets/animations/characters.json"));
    
    assets.loadTexture("dungeon-tiles", resolvePath("assets/textures/dungeon-tiles.png"));
    assets.loadTexture("tile-map", resolvePath("assets/textures/dungeon-tiles.png"));
    TileManager::getInstance().loadAtlasConfig(resolvePath("assets/animations/tile-map.json"));

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

RunState& Game::getRunState() {
    return runState;
}

sf::RenderWindow& Game::getWindow() {
    return renderWindow;
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