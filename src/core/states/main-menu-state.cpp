#include "main-menu-state.hpp"

MainMenuState::MainMenuState(StateManager& manager) : GameState(manager) {
    SettingManager& settings = SettingManager::getInstance();

    mainMenu = root.createChild<UI::HorizontalBox>()
        ->setModeX(UI::SizeMode::Expanded)
        ->setFixedWidth(settings.getWindowWidth())
        ->setModeY(UI::SizeMode::Fixed)
        ->setFixedHeight(100.f)
        ->setAlignmentY(UI::AlignmentY::Top)
        ->setPadding(40.f, 40.f, 40.f, 40.f)
        ->setDistribution(UI::Distribution::SpaceBetween);

    mainMenu->setChildDefaults({
        .modeY = UI::SizeMode::Expanded,
        .fixedWidth = 200.f
    });

    button1   = mainMenu->createChild<UI::Button>("Button-1", "regular");
    button2   = mainMenu->createChild<UI::Button>("Button-2", "regular");
    button3   = mainMenu->createChild<UI::Button>("Button-3", "regular");
    slider    = mainMenu->createChild<UI::Slider>(0.f, 100.f, 50.f);
    textInput = mainMenu->createChild<UI::TextInput>("regular");
}