#include "enemy-factory.hpp"
#include "sprinter.hpp"
#include "soldier.hpp"
#include "brute.hpp"
#include "boss-malachar.hpp"

EnemyType EnemyFactory::stringToEnemyType(const std::string& typeName) {
    if (typeName == "SPRINTER" || typeName == "SPRINTER_CARRIER" || typeName == "BONE_SPRINTER" || typeName == "BONE_SPRINTER_CARRIER" || typeName == "WATERLOGGED_SCRIBE" || typeName == "SHARD_WRAITH" || typeName == "MIRROR_BEARER" || typeName == "MIRROR_BEARER_CARRIER") {
        return EnemyType::SPRINTER;
    }
    if (typeName == "BRUTE" || typeName == "SIEGE_WRAITH" || typeName == "RESONANT_CANTOR" || typeName == "VOID_SHUNTER") {
        return EnemyType::BRUTE;
    }
    if (typeName == "BOSS_MALACHAR") {
        return EnemyType::BOSS_MALACHAR;
    }
    return EnemyType::SOLDIER;
}

std::unique_ptr<Enemy> EnemyFactory::createEnemy(EnemyType type, Player& player) {
    switch (type) {
        case EnemyType::SPRINTER:
            return std::make_unique<Sprinter>(player, false);
        case EnemyType::SOLDIER:
            return std::make_unique<Soldier>(player);
        case EnemyType::BRUTE:
            return std::make_unique<Brute>(player);
        case EnemyType::BOSS_MALACHAR:
            return std::make_unique<BossMalachar>(player);
        default:
            return std::make_unique<Soldier>(player);
    }
}

std::unique_ptr<Enemy> EnemyFactory::createEnemy(const std::string& typeName, Player& player) {
    if (typeName == "SPRINTER_CARRIER" || typeName == "BONE_SPRINTER_CARRIER" || typeName == "CHOIR_HUSK_CARRIER" || typeName == "MIRROR_BEARER_CARRIER") {
        return std::make_unique<Sprinter>(player, true);
    }
    return createEnemy(stringToEnemyType(typeName), player);
}