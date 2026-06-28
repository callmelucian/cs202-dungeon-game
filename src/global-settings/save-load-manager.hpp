#ifndef SAVE_LOAD_MANAGER_HPP
#define SAVE_LOAD_MANAGER_HPP

#include <string>
#include "../core/run-state.hpp"

/**
 * @file save-load-manager.hpp
 * @brief Singleton for saving and loading the game's RunState.
 */

class SaveLoadManager {
public:
    static SaveLoadManager& getInstance();

    SaveLoadManager(const SaveLoadManager&) = delete;
    SaveLoadManager& operator=(const SaveLoadManager&) = delete;

    bool saveGame(const RunState& state, const std::string& filename = "savegame.json");
    bool loadGame(RunState& state, const std::string& filename = "savegame.json");

private:
    SaveLoadManager() = default;
    ~SaveLoadManager() = default;
};

#endif // SAVE_LOAD_MANAGER_HPP
