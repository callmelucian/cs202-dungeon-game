#ifndef ENUMS_HPP
#define ENUMS_HPP

/**
 * @file enums.hpp
 * @brief Shared enumerations for the Echoes of the Ashen Vault game.
 * 
 * Contains globally used enums such as EndingType, EchoType, EchoOutcome,
 * FormType, ChamberType, and EnemyType.
 */

enum class EndingType {
    ENDING_A_SHATTER,
    ENDING_B_RETREAT,
    ENDING_C_WARNING
};

enum class EchoType {
    MARROW,
    HOLLOW_BELL,
    CLARITY_SHARD,
    RESONANCE_CORE,
    OBSIDIAN_KEY
};

enum class EchoOutcome {
    UNCOLLECTED,
    COLLECTED,
    STOLEN
};

enum class FormType {
    WRAITHBLADE,
    VOIDCASTER,
    IRONSHELL
};

enum class ChamberType {
    PROTECT,
    PREVENT,
    GAUNTLET,
    MID,
    BOSS
};

enum class EnemyType {
    SPRINTER,
    SOLDIER,
    BRUTE,
    BOSS_MALACHAR
};

// Game-wide design constants — change here and it propagates everywhere
constexpr float SWITCH_COOLDOWN_DURATION = 4.0f;   // seconds, form switch cooldown
constexpr float MAX_MOMENTUM             = 100.0f;  // momentum cap per form
constexpr float HUD_LERP_SPEED           = 10.0f;   // HUD bar smooth animation speed

#endif // ENUMS_HPP
