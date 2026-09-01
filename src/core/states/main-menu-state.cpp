#include "main-menu-state.hpp"
#include "../../ui/base/text.hpp"
#include "../../ui/containers/flex-box.hpp"

#include "choose-chamber-state.hpp"
#include "tutorial-state.hpp"
#include "setting-state.hpp"
#include "pause-state.hpp"
#include "game-play-state.hpp"
#include "../../global-settings/save-load-manager.hpp"
#include "../game.hpp"
#include <fstream>

MainMenuState::MainMenuState(StateManager& manager) : GameState(manager) {
    SettingManager& settings = SettingManager::getInstance();
    root->setAlignmentY(UI::AlignmentY::Middle);

    // Create a vertical layout box that expands to the full screen
    layoutBox = root->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Expanded)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setSpacing(60.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    // Title Text (header, size 50, auto-sized)
    titleText = layoutBox->createChild<UI::Text>("header", 50)
        ->setString("Echoes of the Ashen Vault");

    // Horizontal Box for buttons (contained to fit children)
    buttonBox = layoutBox->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setSpacing(20.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    // Set defaults for buttons inside buttonBox
    buttonBox->setChildDefaults({
        .modeX = UI::SizeMode::Fixed,
        .modeY = UI::SizeMode::Fixed,
        .fixedWidth = 200.f,
        .fixedHeight = 48.f,
    });

    // Add buttons
    // Check if a save file exists to enable Continue
    bool hasSave = static_cast<bool>(std::ifstream("savegame.json"));

    continueButton = buttonBox->createChild<UI::Button>("Continue", "regular", 24)
        ->setEnabled(hasSave)
        ->setOnClick([this]() {
            RunState& runState = Game::getInstance().getRunState();
            if (SaveLoadManager::getInstance().loadGame(runState)) {
                stateManager.pushState(std::make_unique<GameplayState>(stateManager));
            }
        });
    playButton = buttonBox->createChild<UI::Button>("New Game", "regular", 24)
        ->setOnClick([this]() {
            stateManager.pushState(std::make_unique<ChooseChamberState>(stateManager));
        });
    tutorialButton = buttonBox->createChild<UI::Button>("Tutorial", "regular", 24)
        ->setOnClick([this]() {
            stateManager.pushState(std::make_unique<TutorialState>(stateManager));
        });
    optionsButton = buttonBox->createChild<UI::Button>("Settings", "regular", 24)
        ->setOnClick([this]() {
            stateManager.pushState(std::make_unique<SettingState>(stateManager));
        });
    exitButton = buttonBox->createChild<UI::Button>("Exit", "regular", 24)
        ->setOnClick([this]() {
            Game::getInstance().getWindow().close();
        });
}