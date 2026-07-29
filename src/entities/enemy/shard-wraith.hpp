#ifndef SHARD_WRAITH_HPP
#define SHARD_WRAITH_HPP

#include "enemy.hpp"

/**
 * @brief ShardWraith represents a fast flying minion spawned by BossMalachar in Phase 1.
 * Base Stats: HP 30, Damage 8, Speed 4.0, flying movement towards player.
 */
class ShardWraith : public Enemy {
public:
    ShardWraith(Player& player);
    virtual ~ShardWraith() = default;

    void onDeath(Chamber* chamber = nullptr) override;
};

#endif // SHARD_WRAITH_HPP
