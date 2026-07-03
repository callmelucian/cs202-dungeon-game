#include "main-menu-state.hpp"
#include "../../ui/base/text.hpp"
#include "../../ui/containers/flex-box.hpp"

MainMenuState::MainMenuState(StateManager& manager) : GameState(manager) {
    SettingManager& settings = SettingManager::getInstance();

    // Create a vertical layout box that expands to the full screen
    layoutBox = root->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Expanded)
        ->setModeY(UI::SizeMode::Expanded)
        ->setAlignmentX(UI::AlignmentX::Center)
        ->setDistribution(UI::Distribution::SpaceEvenly);

    // Title Text (bold, size 54, auto-sized)
    titleText = layoutBox->createChild<UI::Text>("bold", 54)
        ->setString("Echoes of the Ashen Vault");

    // Horizontal Box for buttons (contained to fit children)
    buttonBox = layoutBox->createChild<UI::HorizontalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setSpacing(25.f);

    // Set defaults for buttons inside buttonBox
    buttonBox->setChildDefaults({
        .modeX = UI::SizeMode::Fixed,
        .modeY = UI::SizeMode::Fixed,
        .fixedWidth = 180.f,
        .fixedHeight = 50.f
    });

    // Add buttons
    playButton    = buttonBox->createChild<UI::Button>("Play", "regular");
    optionsButton = buttonBox->createChild<UI::Button>("Options", "regular");
    exitButton    = buttonBox->createChild<UI::Button>("Exit", "regular");
}