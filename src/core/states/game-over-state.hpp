#ifndef GAME_OVER_STATE
#define GAME_OVER_STATE

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

// EndingType: Defines the possible narrative/gameplay endings displayed on the game over screen.
enum class EndingType {
    A,
    B,
    C
};

// GameOverState: Represents the state when the game ends, displaying the corresponding EndingType (A, B, or C).
class GameOverState : public GameState {
public:
    GameOverState(StateManager& manager, EndingType endingType);

private:
    EndingType endingType;
    UI::VerticalBox* layoutBox;
    UI::Text* titleText;
    UI::Text* endingText;
    UI::Button* menuButton;
};

#endif // GAME_OVER_STATE
