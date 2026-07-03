#ifndef MAIN_MENU_STATE
#define MAIN_MENU_STATE

#include <SFML/Graphics.hpp>
#include "../game-state.hpp"
#include "../state-manager.hpp"
#include "../game.hpp"
#include "../../global-settings/setting-manager.hpp"
#include <iostream>
#include <memory>

// UI Framework
#include "../../ui/base/component.hpp"
#include "../../ui/containers/container.hpp"
#include "../../ui/widgets/button.hpp"
#include "../../ui/widgets/slider.hpp"
#include "../../ui/widgets/text-input.hpp"

class MainMenuState : public GameState {
public:
    MainMenuState(StateManager& manager);

private:
    UI::VerticalBox* layoutBox;
    UI::Text* titleText;
    UI::HorizontalBox* buttonBox;
    UI::Button* playButton;
    UI::Button* optionsButton;
    UI::Button* exitButton;
};

#endif // MAIN_MENU_STATE