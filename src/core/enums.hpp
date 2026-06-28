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
    WATERLOGGED_SCRIBE,
    SHARD_SOLDIER,
    SIEGE_WRAITH,
    CHOIR_HUSK,
    RESONANT_CANTOR,
    HUSHED_STALKER,
    MIRROR_BEARER,
    VOID_SHUNTER,
    SARCOPHAGUS_WARDEN,
    BOSS_MALACHAR
};

#endif // ENUMS_HPP
