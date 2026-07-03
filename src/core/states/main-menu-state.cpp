#include "main-menu-state.hpp"
#include "../../ui/base/text.hpp"
#include "../../ui/containers/flex-box.hpp"

#include "game-play-state.hpp"
#include "setting-state.hpp"
#include "pause-state.hpp"

MainMenuState::MainMenuState(StateManager& manager) : GameState(manager) {
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
        ->setString("Echoes of the Ashen Vault");

    // Horizontal Box for buttons (contained to fit children)
    buttonBox = layoutBox->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setSpacing(25.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    // Set defaults for buttons inside buttonBox
    buttonBox->setChildDefaults({
        .modeX = UI::SizeMode::Fixed,
        .modeY = UI::SizeMode::Fixed,
        .fixedWidth = 180.f,
        .fixedHeight = 50.f,
    });

    // Add buttons
    playButton = buttonBox->createChild<UI::Button>("Play", "regular", 25)
        ->setOnClick([this]() {
            stateManager.pushState(std::make_unique<GameplayState>(stateManager));
        });
    optionsButton = buttonBox->createChild<UI::Button>("Settings", "regular", 25)
        ->setOnClick([this]() {
            stateManager.pushState(std::make_unique<SettingState>(stateManager));
        });
    exitButton = buttonBox->createChild<UI::Button>("Exit", "regular", 25)
        ->setOnClick([this]() {
            Game::getInstance().getWindow().close();
        });
}