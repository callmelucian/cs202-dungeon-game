#ifndef GAME_PLAY_STATE
#define GAME_PLAY_STATE

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

// GameplayState: Manages the active game session, processing entity updates, level progression, and combat logic.
class GameplayState : public GameState {
public:
    GameplayState(StateManager& manager);

private:
    UI::VerticalBox* layoutBox;
    UI::Text* titleText;
    UI::HorizontalBox* buttonBox;
    UI::Button* pauseButton;
    UI::Button* quitButton;
};

#endif // GAME_PLAY_STATE
