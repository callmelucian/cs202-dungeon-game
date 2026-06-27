#ifndef GAME_PLAY_STATE
#define GAME_PLAY_STATE

#include <SFML/Graphics.hpp>
#include "../game-state.hpp"
#include "../state-manager.hpp"
#include "../game.hpp"
#include "../../global-settings/setting-manager.hpp"
#include <iostream>
#include <memory>

// GameplayState: Manages the active game session, processing entity updates, level progression, and combat logic.
class GameplayState : public GameState {
public:
    GameplayState(StateManager& manager);
};

#endif // GAME_PLAY_STATE
