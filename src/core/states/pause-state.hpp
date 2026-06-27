#ifndef PAUSE_STATE
#define PAUSE_STATE

#include <SFML/Graphics.hpp>
#include "../game-state.hpp"
#include "../state-manager.hpp"
#include "../game.hpp"
#include "../../global-settings/setting-manager.hpp"
#include <iostream>
#include <memory>

// PauseState: Represents the overlay menu shown when the game is paused, allowing the player to resume or quit.
class PauseState : public GameState {
public:
    PauseState(StateManager& manager);
};

#endif // PAUSE_STATE
