#include "setting-state.hpp"
#include "../../global-settings/sound-manager.hpp"

SettingState::SettingState(StateManager& manager) : GameState(manager) {
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
        ->setString("Settings");

    // Vertical Box for buttons/sliders (contained to fit children)
    buttonBox = layoutBox->createChild<UI::VerticalBox>()
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained)
        ->setSpacing(25.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    // Set defaults for widgets inside buttonBox (sliders and buttons)
    buttonBox->setChildDefaults({
        .modeX = UI::SizeMode::Fixed,
        .modeY = UI::SizeMode::Fixed,
        .fixedWidth = 200.f,
        .fixedHeight = 50.f
    });

    // Add labels and widgets
    // Text labels override the default Fixed size to be Contained so they display properly
    musicLabel = buttonBox->createChild<UI::Text>("regular", 20)
        ->setString("Music Volume")
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained);

    musicSlider = buttonBox->createChild<UI::Slider>(0.f, 100.f, settings.getMusicVolume())
        ->setOnValueChanged([](float val) {
            SettingManager::getInstance().setMusicVolume(val);
            SoundManager::getInstance().setMusicVolume(val);
        });

    sfxLabel = buttonBox->createChild<UI::Text>("regular", 20)
        ->setString("SFX Volume")
        ->setModeX(UI::SizeMode::Contained)
        ->setModeY(UI::SizeMode::Contained);

    sfxSlider = buttonBox->createChild<UI::Slider>(0.f, 100.f, settings.getSfxVolume())
        ->setOnValueChanged([](float val) {
            SettingManager::getInstance().setSfxVolume(val);
            SoundManager::getInstance().setSfxVolume(val);
        });

    backButton = buttonBox->createChild<UI::Button>("Back", "regular", 25.f)
        ->setOnClick([this]() {
            stateManager.popState();
        });
}
