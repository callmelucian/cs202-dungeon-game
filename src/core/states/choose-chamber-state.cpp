#include "choose-chamber-state.hpp"
#include "individual-chamber-state.hpp"
#include "game-play-state.hpp"
#include "main-menu-state.hpp"
#include "../game.hpp"
#include "../../global-settings/save-load-manager.hpp"

ChooseChamberState::ChooseChamberState(StateManager& manager) : GameState(manager) {
    root->setAlignmentY(UI::AlignmentY::Middle);

    // Create a vertical layout box that expands to the full screen
    layoutBox = root->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Expanded)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setSpacing(60.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    // Title Text (header, size 42, auto-sized)
    titleText = layoutBox->createChild<UI::Text>("header", 42)
        ->setString("New Game");

    // Vertical Box for buttons (contained to fit children)
    buttonBox = layoutBox->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setSpacing(25.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    // Set defaults for buttons inside buttonBox
    buttonBox->setChildDefaults({
        .modeX = UI::SizeMode::Fixed,
        .modeY = UI::SizeMode::Fixed,
        .fixedWidth = 280.f,
        .fixedHeight = 55.f,
    });

    // 1. Story Mode Button
    storyModeBtn = buttonBox->createChild<UI::Button>("Story Mode", "regular", 24)
        ->setOnClick([this]() {
            RunState& runState = Game::getInstance().getRunState();
            runState = RunState{}; // Fresh baseline run
            runState.currentLevel = 1;
            runState.currentChamber = 1;
            SaveLoadManager::getInstance().saveGame(runState);
            stateManager.pushState(std::make_unique<GameplayState>(stateManager));
        });

    // 2. Individual Chamber Button
    individualChamberBtn = buttonBox->createChild<UI::Button>("Individual Chamber", "regular", 24)
        ->setOnClick([this]() {
            stateManager.pushState(std::make_unique<IndividualChamberState>(stateManager));
        });
        
    // 3. Back Button
    backBtn = buttonBox->createChild<UI::Button>("Back", "regular", 24)
        ->setOnClick([this]() {
            stateManager.popState();
        });
}
