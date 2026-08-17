#include "soldier.hpp"
#include "enemy-state.hpp"
#include "enemy-steering-strategy.hpp"
#include "../player.hpp"
#include <iostream>

Soldier::Soldier(Player& player) : Enemy("soldier", player) {
    baseStats.maxHp = 22.0f;
    baseStats.hp = 22.0f;
    baseStats.damage = 8.0f;
    baseStats.speed = 3.5f;

    setSteeringStrategy(std::make_unique<SeekStrategy>());
    changeState(std::make_unique<ChasingState>());
}

void Soldier::onDeath(Chamber* chamber) {
    std::cout << "Soldier defeated.\n";
}
