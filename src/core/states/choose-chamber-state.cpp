#include "choose-chamber-state.hpp"
#include "game-play-state.hpp"
#include "main-menu-state.hpp"
#include "../../chambers/chamber-factory.hpp"

ChooseChamberState::ChooseChamberState(StateManager& manager) : GameState(manager) {
    root->setAlignmentY(UI::AlignmentY::Middle);

    // Create a vertical layout box that expands to the full screen
    layoutBox = root->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Expanded)
        ->setModeY(UI::SizeMode::Contained)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setSpacing(50.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    // Title Text (bold, size 54, auto-sized)
    titleText = layoutBox->createChild<UI::Text>("bold", 54)
        ->setString("Select Chamber");

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
        .fixedWidth = 250.f,
        .fixedHeight = 50.f,
    });

    // Add buttons
    testChamberBtn = buttonBox->createChild<UI::Button>("Test Chamber", "regular", 25)
        ->setOnClick([this]() {
            stateManager.pushState(std::make_unique<GameplayState>(stateManager, ChamberSelectionType::TEST));
        });
        
    protectChamberBtn = buttonBox->createChild<UI::Button>("Protect Chamber", "regular", 25)
        ->setOnClick([this]() {
            stateManager.pushState(std::make_unique<GameplayState>(stateManager, ChamberSelectionType::PROTECT));
        });
        
    preventChamberBtn = buttonBox->createChild<UI::Button>("Prevent Chamber", "regular", 25)
        ->setOnClick([this]() {
            stateManager.pushState(std::make_unique<GameplayState>(stateManager, ChamberSelectionType::PREVENT));
        });
        
    backBtn = buttonBox->createChild<UI::Button>("Back", "regular", 25)
        ->setOnClick([this]() {
            stateManager.popState();
        });
}
