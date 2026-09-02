#include "pause-state.hpp"
#include "main-menu-state.hpp"
#include "setting-state.hpp"
#include "echo-log-state.hpp"
#include "../../global-settings/save-load-manager.hpp"
#include "../game.hpp"

PauseState::PauseState(StateManager& manager) : GameState(manager) {
    SettingManager& settings = SettingManager::getInstance();
    root->setAlignmentY(UI::AlignmentY::Middle);

    // Create a vertical layout box that expands to the full screen
    layoutBox = root->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Expanded)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setSpacing(35.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    // Title Text (header, size 36, auto-sized)
    titleText = layoutBox->createChild<UI::Text>("header", 36)
        ->setString("Game Paused");

    // Vertical Box for buttons (contained to fit children)
    buttonBox = layoutBox->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setSpacing(20.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    // Set defaults for buttons inside buttonBox
    buttonBox->setChildDefaults({
        .modeX = UI::SizeMode::Fixed,
        .modeY = UI::SizeMode::Fixed,
        .fixedWidth = 220.f,
        .fixedHeight = 48.f
    });

    // Add buttons
    resumeButton = buttonBox->createChild<UI::Button>("Resume", "regular", 24.f)
        ->setOnClick([this]() {
            stateManager.popState();
        });
    echoButton = buttonBox->createChild<UI::Button>("Echo Log", "regular", 24.f)
        ->setOnClick([this]() {
            stateManager.pushState(std::make_unique<EchoLogState>(stateManager));
        });
    saveButton = buttonBox->createChild<UI::Button>("Save Game", "regular", 24.f)
        ->setOnClick([this]() {
            SaveLoadManager::getInstance().saveGame(Game::getInstance().getRunState());
        });
    settingButton = buttonBox->createChild<UI::Button>("Settings", "regular", 24.f)
        ->setOnClick([this]() {
            stateManager.pushState(std::make_unique<SettingState>(stateManager));
        });
    menuButton = buttonBox->createChild<UI::Button>("Main Menu", "regular", 24.f)
        ->setOnClick([this]() {
            stateManager.clearAndSetState(std::make_unique<MainMenuState>(stateManager));
        });
}

void PauseState::handleEvents(sf::Event& event) {
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->scancode == sf::Keyboard::Scancode::Escape) {
            stateManager.popState();
            return;
        }
    }
    GameState::handleEvents(event);
}
