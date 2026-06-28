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

    // Foretell flags from Clarity Shard
    bool foretellActive = false;
    bool foretellPhase1 = false;
};

#endif // RUN_STATE_HPP
