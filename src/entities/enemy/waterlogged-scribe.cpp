#include "waterlogged-scribe.hpp"
#include "enemy-state.hpp"
#include "enemy-steering-strategy.hpp"
#include <iostream>

WaterloggedScribe::WaterloggedScribe(Player& player) : Enemy("waterlogged_scribe", player)  {
    baseStats.hp = 18.0f;
    baseStats.maxHp = 18.0f;
    baseStats.damage = 5.0f;
    baseStats.speed = 1.5f;
    fragmentDropCount = 1;
    
    attackCooldown = 0.0f;

    setSteeringStrategy(std::make_unique<SeekStrategy>());

    changeState(std::make_unique<IdleState>());
}

void WaterloggedScribe::draw(sf::RenderWindow& window) const {
    // Wait for animation to be built
}

// PLaceholder, will update later
void WaterloggedScribe::onDeath() {
    std::cout << "WaterloggedScribe died and dropped " << fragmentDropCount << " Fragment(s)!\n";
}
