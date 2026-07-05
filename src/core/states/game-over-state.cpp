#include "game-over-state.hpp"
#include "main-menu-state.hpp"
#include <string>

GameOverState::GameOverState(StateManager& manager, EndingType endingType)
    : GameState(manager), endingType(endingType) {
    SettingManager& settings = SettingManager::getInstance();
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
        ->setString("Game Over");

    // Subtitle / Ending Text (italic, size 24, auto-sized)
    std::string endingStr;
    switch (endingType) {
        case EndingType::ENDING_A_SHATTER:
            endingStr = "Ending A: The Vault Collapses";
            break;
        case EndingType::ENDING_B_RETREAT:
            endingStr = "Ending B: Ascended to Light";
            break;
        case EndingType::ENDING_C_WARNING:
            endingStr = "Ending C: Lost in the Dark";
            break;
    }
    endingText = layoutBox->createChild<UI::Text>("italic", 24)
        ->setString(endingStr);

    // Back-to-main-menu button
    menuButton = layoutBox->createChild<UI::Button>("Back to Main Menu", "regular", 25)
        ->setModeX(UI::SizeMode::Fixed)
        ->setModeY(UI::SizeMode::Fixed)
        ->setFixedWidth(280.f)
        ->setFixedHeight(50.f)
        ->setOnClick([this]() {
            stateManager.clearAndSetState(std::make_unique<MainMenuState>(stateManager));
        });
}
