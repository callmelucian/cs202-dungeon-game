#include "game-over-state.hpp"

GameOverState::GameOverState(StateManager& manager, EndingType endingType)
    : GameState(manager), endingType(endingType) {
    // Empty state without UI components
}
