#include "shard-soldier.hpp"
#include "enemy-state.hpp"
#include "enemy-steering-strategy.hpp"
#include "../../core/game.hpp"
#include <iostream>

ShardSoldier::ShardSoldier(Player& player) 
    : Enemy("shard_soldier", player), selfHealActive(false) 
{
    baseStats.hp = 16.0f;
    baseStats.maxHp = 16.0f;
    baseStats.damage = 6.0f;
    baseStats.speed = 3.5f; 
    fragmentDropCount = 1;
    
    attackCooldown = 0.0f;

    setSteeringStrategy(std::make_unique<SeekStrategy>());
    changeState(std::make_unique<IdleState>());
}


// Placeholder, will update later
void ShardSoldier::onDeath(Chamber* chamber) {
    std::cout << "ShardSoldier died and dropped " << fragmentDropCount << " Fragment(s)!\n";
}

void ShardSoldier::update(float deltaTime) {
    Enemy::update(deltaTime);

    // Marrow Echo Stolen Modifier: Non-Siege enemies self-heal 3% Max HP/s
    RunState& runState = Game::getInstance().getRunState();
    auto marrowIt = runState.echoOutcomes.find(EchoType::MARROW);
    if (selfHealActive || (marrowIt != runState.echoOutcomes.end() && marrowIt->second == EchoOutcome::STOLEN)) {
        applySelfHeal(deltaTime);
    }
}

void ShardSoldier::applySelfHeal(float dt) {
    if (baseStats.hp < baseStats.maxHp) {
        baseStats.hp += (0.03f * baseStats.maxHp) * dt;
        
        if (baseStats.hp > baseStats.maxHp) {
            baseStats.hp = baseStats.maxHp;
        }
    }
}

void ShardSoldier::setSelfHealActive(bool active) {
    selfHealActive = active;
}

bool ShardSoldier::isSelfHealActive() const {
    return selfHealActive;
}