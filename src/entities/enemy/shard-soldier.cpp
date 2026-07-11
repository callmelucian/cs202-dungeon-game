#include "shard-soldier.hpp"
#include "enemy-state.hpp"
#include "enemy-steering-strategy.hpp"
#include <iostream>

ShardSoldier::ShardSoldier(Player& player) 
    : Enemy("shard_soldier", player), selfHealActive(false) 
{
    baseStats.hp = 16.0f;
    baseStats.maxHp = 16.0f;
    baseStats.damage = 6.0f;
    baseStats.speed = 5.0f; 
    fragmentDropCount = 1;
    
    attackCooldown = 0.0f;

    setSteeringStrategy(std::make_unique<SeekStrategy>());
    changeState(new IdleState());
}

void ShardSoldier::draw(sf::RenderWindow& window) const {
    // Wait for animation to be built
}

// Placeholder, will update later
void ShardSoldier::onDeath() {
    std::cout << "ShardSoldier died and dropped " << fragmentDropCount << " Fragment(s)!\n";
}

void ShardSoldier::update(float deltaTime) {
    Enemy::update(deltaTime);

    if (selfHealActive) {
        applySelfHeal(deltaTime);
    }
}

void ShardSoldier::applySelfHeal(float dt) {
    if (baseStats.hp < baseStats.maxHp) {
        baseStats.hp += 2.0f * dt;
        
        if (baseStats.hp > baseStats.maxHp) {
            baseStats.hp = baseStats.maxHp;
        }
    }
}