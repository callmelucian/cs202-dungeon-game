#include "pause-state.hpp"
#include "main-menu-state.hpp"
#include "setting-state.hpp"
#include "../../global-settings/save-load-manager.hpp"
#include "../game.hpp"
#include <iostream>

PauseState::PauseState(StateManager& manager) : GameState(manager) {
    SettingManager& settings = SettingManager::getInstance();
    root->setAlignmentY(UI::AlignmentY::Middle);

    // Create a vertical layout box that expands to the full screen
    layoutBox = root->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Expanded)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setSpacing(50.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    // Title Text (header, size 36, auto-sized)
    titleText = layoutBox->createChild<UI::Text>("header", 36)
        ->setString("Game Paused");

    // Vertical Box for buttons (contained to fit children)
    buttonBox = layoutBox->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setSpacing(25.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    // Set defaults for buttons inside buttonBox
    buttonBox->setChildDefaults({
        .modeX = UI::SizeMode::Fixed,
        .modeY = UI::SizeMode::Fixed,
        .fixedWidth = 200.f,
        .fixedHeight = 50.f
    });

    // Add buttons
    resumeButton = buttonBox->createChild<UI::Button>("Resume", "regular", 25.f)
        ->setOnClick([this]() {
            stateManager.popState();
        });
    saveButton = buttonBox->createChild<UI::Button>("Save Game", "regular", 25.f)
        ->setOnClick([this]() {
            bool ok = SaveLoadManager::getInstance().saveGame(Game::getInstance().getRunState());
            if (ok) {
                std::cout << "Game saved successfully.\n";
            }
        });
    settingButton = buttonBox->createChild<UI::Button>("Settings", "regular", 25.f)
        ->setOnClick([this]() {
            stateManager.pushState(std::make_unique<SettingState>(stateManager));
        });
    menuButton = buttonBox->createChild<UI::Button>("Main Menu", "regular", 25.f)
        ->setOnClick([this]() {
            stateManager.clearAndSetState(std::make_unique<MainMenuState>(stateManager));
        });
}
