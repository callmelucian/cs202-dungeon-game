#include "save-load-manager.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

SaveLoadManager& SaveLoadManager::getInstance() {
    static SaveLoadManager instance;
    return instance;
}

bool SaveLoadManager::saveGame(const RunState& state, const std::string& filename) {
    try {
        json j;
        j["currentLevel"] = state.currentLevel;
        j["currentChamber"] = state.currentChamber;

        json outcomes;
        for (const auto& [type, outcome] : state.echoOutcomes) {
            outcomes[std::to_string(static_cast<int>(type))] = static_cast<int>(outcome);
        }
        j["echoOutcomes"] = outcomes;
        j["echoesStolen"] = state.echoesStolen;
        
        j["playerHP"] = state.playerHP;
        j["activeForm"] = static_cast<int>(state.activeForm);
        
        j["wraithbladeMomentum"] = state.wraithbladeMomentum;
        j["voidcasterMomentum"] = state.voidcasterMomentum;
        j["ironshellMomentum"] = state.ironshellMomentum;

        j["collectTimeReduction"] = state.collectTimeReduction;
        j["special1MomentumThreshold"] = state.special1MomentumThreshold;
        j["special2MomentumThreshold"] = state.special2MomentumThreshold;

        j["foretellActive"] = state.foretellActive;
        j["foretellPhase1"] = state.foretellPhase1;

        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "SaveLoadManager: Could not open file for writing: " << filename << std::endl;
            return false;
        }
        
        file << j.dump(4);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "SaveLoadManager: Failed to save game: " << e.what() << std::endl;
        return false;
    }
}

bool SaveLoadManager::loadGame(RunState& state, const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "SaveLoadManager: Save file not found: " << filename << std::endl;
            return false;
        }

        json j;
        file >> j;

        if (j.contains("currentLevel")) state.currentLevel = j["currentLevel"];
        if (j.contains("currentChamber")) state.currentChamber = j["currentChamber"];

        if (j.contains("echoOutcomes")) {
            for (auto& el : j["echoOutcomes"].items()) {
                EchoType type = static_cast<EchoType>(std::stoi(el.key()));
                EchoOutcome outcome = static_cast<EchoOutcome>(el.value().get<int>());
                state.echoOutcomes[type] = outcome;
            }
        }
        if (j.contains("echoesStolen")) state.echoesStolen = j["echoesStolen"];

        if (j.contains("playerHP")) state.playerHP = j["playerHP"];
        if (j.contains("activeForm")) state.activeForm = static_cast<FormType>(j["activeForm"].get<int>());

        if (j.contains("wraithbladeMomentum")) state.wraithbladeMomentum = j["wraithbladeMomentum"];
        if (j.contains("voidcasterMomentum")) state.voidcasterMomentum = j["voidcasterMomentum"];
        if (j.contains("ironshellMomentum")) state.ironshellMomentum = j["ironshellMomentum"];

        if (j.contains("collectTimeReduction")) state.collectTimeReduction = j["collectTimeReduction"];
        if (j.contains("special1MomentumThreshold")) state.special1MomentumThreshold = j["special1MomentumThreshold"];
        if (j.contains("special2MomentumThreshold")) state.special2MomentumThreshold = j["special2MomentumThreshold"];

        if (j.contains("foretellActive")) state.foretellActive = j["foretellActive"];
        if (j.contains("foretellPhase1")) state.foretellPhase1 = j["foretellPhase1"];

        return true;
    } catch (const std::exception& e) {
        std::cerr << "SaveLoadManager: Failed to load game: " << e.what() << std::endl;
        return false;
    }
}
