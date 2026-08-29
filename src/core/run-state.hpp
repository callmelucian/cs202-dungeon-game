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

    // Echo power percentage tracking (0.0 to 100.0%)
    std::map<EchoType, float> echoPowers = {
        {EchoType::MARROW, 0.0f},
        {EchoType::HOLLOW_BELL, 0.0f},
        {EchoType::CLARITY_SHARD, 0.0f},
        {EchoType::RESONANCE_CORE, 0.0f},
        {EchoType::OBSIDIAN_KEY, 0.0f}
    };

    // Number of echoes stolen for ending calculation
    int echoesStolen = 0;

    // Player state
    float playerHP = 100.0f;
    FormType activeForm = FormType::WRAITHBLADE;

    // L3Ch4 Decoy Reliquary defended buff (+20% Max HP for final chamber)
    bool hasDecoyReliquaryBuff = false;

    // Per-form momentum
    float wraithbladeMomentum = 0.0f;
    float voidcasterMomentum = 0.0f;
    float ironshellMomentum = 0.0f;

    // Modifiers from Echoes
    float collectTimeReduction = 1.0f; // Clarity Shard multiplier (0.80 if intact, 0.90 if collected)
    float special1MomentumThreshold = 50.0f; // Hollow Bell threshold (35 if intact, 42.5 if collected)
    float special2MomentumThreshold = 100.0f; // Resonance Core threshold

    // Foretell flags from Clarity Shard
    bool foretellActive = false;
    bool foretellPhase1 = false;

    void syncEchoModifiers() {
        auto hollowIt = echoOutcomes.find(EchoType::HOLLOW_BELL);
        if (hollowIt != echoOutcomes.end() && hollowIt->second == EchoOutcome::COLLECTED) {
            float power = echoPowers.count(EchoType::HOLLOW_BELL) ? echoPowers[EchoType::HOLLOW_BELL] : 100.0f;
            if (power >= 90.0f) {
                special1MomentumThreshold = 35.0f; // 30% reduction if fully intact
            } else {
                special1MomentumThreshold = 42.5f; // 15% standard reduction
            }
        } else {
            special1MomentumThreshold = 50.0f;
        }

        auto clarityIt = echoOutcomes.find(EchoType::CLARITY_SHARD);
        if (clarityIt != echoOutcomes.end() && clarityIt->second == EchoOutcome::COLLECTED) {
            float power = echoPowers.count(EchoType::CLARITY_SHARD) ? echoPowers[EchoType::CLARITY_SHARD] : 100.0f;
            foretellActive = true;
            if (power >= 90.0f) {
                collectTimeReduction = 0.80f; // 20% collection time reduction
                foretellPhase1 = true;         // Foretell begins in Phase 1
            } else {
                collectTimeReduction = 0.90f; // 10% collection time reduction
                foretellPhase1 = false;        // Foretell begins in Phase 2
            }
        } else {
            collectTimeReduction = 1.0f;
            foretellActive = false;
            foretellPhase1 = false;
        }
    }
};

#endif // RUN_STATE_HPP
