#include "enemy-factory.hpp"
#include "waterlogged-scribe.hpp"
#include "shard-soldier.hpp"
#include "bone-sprinter.hpp"

std::unique_ptr<Enemy> EnemyFactory::createEnemy(EnemyType type, Player& player) {
    switch (type) {
        case EnemyType::WATERLOGGED_SCRIBE:
            return std::make_unique<WaterloggedScribe>(player);
            
        case EnemyType::SHARD_SOLDIER:
            return std::make_unique<ShardSoldier>(player);
            
        case EnemyType::BONE_SPRINTER:
            return std::make_unique<BoneSprinter>(player, false);
            
        default:
            return nullptr;
    }
}