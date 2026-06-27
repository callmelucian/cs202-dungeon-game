#include "main-menu-state.hpp"

MainMenuState::MainMenuState(StateManager& manager) : GameState(manager) {
    SettingManager& settings = SettingManager::getInstance();

    mainMenu = root.createChild<UI::HorizontalBox>();
    mainMenu->setModeX(UI::SizeMode::Expanded);
    mainMenu->setFixedWidth(settings.getWindowWidth());
    mainMenu->setModeY(UI::SizeMode::Fixed);
    mainMenu->setFixedHeight(100.f);
    mainMenu->setAlignmentY(UI::AlignmentY::Top);
    mainMenu->setPadding(40.f, 40.f, 40.f, 40.f);
    mainMenu->setDistribution(UI::Distribution::SpaceBetween);

    button1 = mainMenu->createChild<UI::Button>("Button-1", "regular");
    button1->setModeY(UI::SizeMode::Expanded);
    button1->setFixedWidth(200.f);
    button2 = mainMenu->createChild<UI::Button>("Button-2", "regular");
    button2->setModeY(UI::SizeMode::Expanded);
    button2->setFixedWidth(200.f);
    button3 = mainMenu->createChild<UI::Button>("Button-3", "regular");
    button3->setModeY(UI::SizeMode::Expanded);
    button3->setFixedWidth(200.f);

    slider = mainMenu->createChild<UI::Slider>(0.f, 100.f, 50.f);
    slider->setModeY(UI::SizeMode::Expanded);
    slider->setFixedWidth(200.f);

    textInput = mainMenu->createChild<UI::TextInput>("regular");
    textInput->setModeY(UI::SizeMode::Expanded);
    slider->setFixedWidth(200.f);
}