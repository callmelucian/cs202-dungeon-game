#include "shard-wraith.hpp"
#include "enemy-state.hpp"
#include "enemy-steering-strategy.hpp"
#include "../../chambers/chamber.hpp"
#include "../player.hpp"
#include <iostream>

ShardWraith::ShardWraith(Player& player)
    : Enemy("shard_wraith", player) {
    baseStats.hp = 30.0f;
    baseStats.maxHp = 30.0f;
    baseStats.damage = 8.0f;
    baseStats.speed = 4.0f;
    fragmentDropCount = 1;

    setSteeringStrategy(std::make_unique<SeekStrategy>());
    changeState(std::make_unique<IdleState>());
}

void ShardWraith::onDeath(Chamber* chamber) {
    std::cout << "[ShardWraith] Shard Wraith destroyed!\n";
}
