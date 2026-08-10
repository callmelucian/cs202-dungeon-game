#include "enemy-factory.hpp"
#include "waterlogged-scribe.hpp"
#include "shard-soldier.hpp"
#include "bone-sprinter.hpp"
#include "shard-wraith.hpp"
#include "siege-wraith.hpp"
#include "choir-husk.hpp"
#include "resonant-cantor.hpp"
#include "hushed-stalker.hpp"
#include "mirror-bearer.hpp"
#include "void-shunter.hpp"
#include "boss-malachar.hpp"

EnemyType EnemyFactory::stringToEnemyType(const std::string& typeName) {
    if (typeName == "WATERLOGGED_SCRIBE") return EnemyType::WATERLOGGED_SCRIBE;
    if (typeName == "SHARD_SOLDIER") return EnemyType::SHARD_SOLDIER;
    if (typeName == "BONE_SPRINTER") return EnemyType::BONE_SPRINTER;
    if (typeName == "SHARD_WRAITH") return EnemyType::SHARD_WRAITH;
    if (typeName == "SIEGE_WRAITH") return EnemyType::SIEGE_WRAITH;
    if (typeName == "CHOIR_HUSK") return EnemyType::CHOIR_HUSK;
    if (typeName == "RESONANT_CANTOR") return EnemyType::RESONANT_CANTOR;
    if (typeName == "HUSHED_STALKER") return EnemyType::HUSHED_STALKER;
    if (typeName == "MIRROR_BEARER") return EnemyType::MIRROR_BEARER;
    if (typeName == "VOID_SHUNTER") return EnemyType::VOID_SHUNTER;
    if (typeName == "BOSS_MALACHAR") return EnemyType::BOSS_MALACHAR;
    return EnemyType::WATERLOGGED_SCRIBE;
}

std::unique_ptr<Enemy> EnemyFactory::createEnemy(EnemyType type, Player& player) {
    switch (type) {
        case EnemyType::WATERLOGGED_SCRIBE:
            return std::make_unique<WaterloggedScribe>(player);
        case EnemyType::SHARD_SOLDIER:
            return std::make_unique<ShardSoldier>(player);
        case EnemyType::BONE_SPRINTER:
            return std::make_unique<BoneSprinter>(player, false);
        case EnemyType::SHARD_WRAITH:
            return std::make_unique<ShardWraith>(player);
        case EnemyType::SIEGE_WRAITH:
            return std::make_unique<SiegeWraith>(player);
        case EnemyType::CHOIR_HUSK:
            return std::make_unique<ChoirHusk>(player);
        case EnemyType::RESONANT_CANTOR:
            return std::make_unique<ResonantCantor>(player);
        case EnemyType::HUSHED_STALKER:
            return std::make_unique<HushedStalker>(player);
        case EnemyType::MIRROR_BEARER:
            return std::make_unique<MirrorBearer>(player, MirrorVariant::GUARD);
        case EnemyType::VOID_SHUNTER:
            return std::make_unique<VoidShunter>(player);
        case EnemyType::BOSS_MALACHAR:
            return std::make_unique<BossMalachar>(player);
        default:
            return std::make_unique<WaterloggedScribe>(player);
    }
}

std::unique_ptr<Enemy> EnemyFactory::createEnemy(const std::string& typeName, Player& player) {
    if (typeName == "BONE_SPRINTER_CARRIER") {
        return std::make_unique<BoneSprinter>(player, true);
    } else if (typeName == "CHOIR_HUSK_CARRIER") {
        return std::make_unique<ChoirHusk>(player, HuskVariant::CARRIER);
    } else if (typeName == "CHOIR_HUSK_GUARD") {
        return std::make_unique<ChoirHusk>(player, HuskVariant::GUARD);
    } else if (typeName == "MIRROR_BEARER_CARRIER") {
        return std::make_unique<MirrorBearer>(player, MirrorVariant::CARRIER);
    }
    return createEnemy(stringToEnemyType(typeName), player);
}