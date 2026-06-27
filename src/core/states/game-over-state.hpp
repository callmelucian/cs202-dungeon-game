#ifndef GAME_OVER_STATE
#define GAME_OVER_STATE

#include <SFML/Graphics.hpp>
#include "../game-state.hpp"
#include "../state-manager.hpp"
#include "../game.hpp"
#include "../../global-settings/setting-manager.hpp"
#include <iostream>
#include <memory>

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
};

#endif // GAME_OVER_STATE
