#ifndef RUN_STATE_HPP
#define RUN_STATE_HPP

#include "enums.hpp"
#include <map>

/**
 * @file run-state.hpp
 * @brief Plain serialisable struct that is the single source of truth for persistent run data.
 */

struct RunState {
    int currentLevel = 1;
    int currentChamber = 1;

    // Echo states
    std::map<EchoType, EchoOutcome> echoOutcomes = {
        {EchoType::MARROW, EchoOutcome::UNCOLLECTED},
        {EchoType::HOLLOW_BELL, EchoOutcome::UNCOLLECTED},
        {EchoType::CLARITY_SHARD, EchoOutcome::UNCOLLECTED},
        {EchoType::RESONANCE_CORE, EchoOutcome::UNCOLLECTED},
        {EchoType::OBSIDIAN_KEY, EchoOutcome::UNCOLLECTED}
    };

    // Number of echoes stolen for ending calculation
    int echoesStolen = 0;

    // Player state
    float playerHP = 100.0f;
    FormType activeForm = FormType::WRAITHBLADE;

    // Per-form momentum
    float wraithbladeMomentum = 0.0f;
    float voidcasterMomentum = 0.0f;
    float ironshellMomentum = 0.0f;

    // Modifiers from Echoes
    float collectTimeReduction = 1.0f; // Clarity Shard multiplier
    float special1MomentumThreshold = 50.0f; // Hollow Bell threshold
    float special2MomentumThreshold = 100.0f; // Resonance Core threshold

    // Foretell flags from Clarity Shard
    bool foretellActive = false;
    bool foretellPhase1 = false;

    void syncEchoModifiers() {
        auto hollowIt = echoOutcomes.find(EchoType::HOLLOW_BELL);
        if (hollowIt != echoOutcomes.end()) {
            if (hollowIt->second == EchoOutcome::COLLECTED) {
                special1MomentumThreshold = 42.5f;
            } else {
                special1MomentumThreshold = 50.0f;
            }
        }

        auto clarityIt = echoOutcomes.find(EchoType::CLARITY_SHARD);
        if (clarityIt != echoOutcomes.end()) {
            if (clarityIt->second == EchoOutcome::COLLECTED) {
                collectTimeReduction = 0.9f;
            } else {
                collectTimeReduction = 1.0f;
            }
        }
    }
};

#endif // RUN_STATE_HPP
