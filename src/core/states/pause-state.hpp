#ifndef PAUSE_STATE
#define PAUSE_STATE

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
#include "../../ui/containers/flex-box.hpp"
#include "../../ui/widgets/button.hpp"
#include "../../ui/base/text.hpp"

// PauseState: Represents the overlay menu shown when the game is paused, allowing the player to resume or quit.
class PauseState : public GameState {
public:
    PauseState(StateManager& manager);

private:
    UI::VerticalBox* layoutBox;
    UI::Text* titleText;
    UI::VerticalBox* buttonBox;
    UI::Button* resumeButton;
    UI::Button* menuButton;
};

#endif // PAUSE_STATE
