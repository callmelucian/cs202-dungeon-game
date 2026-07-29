#include "boss-malachar.hpp"
#include "../../chambers/boss-chamber.hpp"
#include "enemy-state.hpp"
#include "enemy-steering-strategy.hpp"
#include "../player.hpp"
#include <iostream>

BossMalachar::BossMalachar(Player& player)
    : Enemy("boss_malachar", player),
      currentPhase(1),
      phaseTimer(0.0f),
      attackTimer(0.0f),
      sunderCooldown(15.0f),
      soulLanceCooldown(10.0f)
{
    baseStats.hp = 1250.0f;
    baseStats.maxHp = 1250.0f;
    baseStats.damage = 14.0f;
    baseStats.speed = 2.0f;
    fragmentDropCount = 10;

    setSteeringStrategy(std::make_unique<SeekStrategy>());
    changeState(std::make_unique<IdleState>());
}

void BossMalachar::update(float deltaTime) {
    Enemy::update(deltaTime);
}

void BossMalachar::updateState(float dt, Chamber& chamber) {
    // Check HP threshold triggers for phase transitions
    if (baseStats.hp <= baseStats.maxHp * 0.75f && currentPhase < 2) {
        transitionPhase(2);
    } else if (baseStats.hp <= baseStats.maxHp * 0.50f && currentPhase < 3) {
        transitionPhase(3);
    } else if (baseStats.hp <= baseStats.maxHp * 0.25f && currentPhase < 4) {
        transitionPhase(4);
    }

    // Dynamic cast to BossChamber if needed for chamber state synchronization
    BossChamber* bossChamber = dynamic_cast<BossChamber*>(&chamber);
    if (bossChamber && bossChamber->getCurrentPhase() != currentPhase) {
        bossChamber->setCurrentPhase(currentPhase);
    }

    // Phase 3 & Phase 4: Platform Sunder attack every 15s
    if (currentPhase >= 3) {
        sunderCooldown -= dt;
        if (sunderCooldown <= 0.0f) {
            platformSunder(chamber);
            sunderCooldown = 15.0f;
        }
    }

    // Phase 4: Soul Lance attack every 10s
    if (currentPhase == 4) {
        soulLanceCooldown -= dt;
        if (soulLanceCooldown <= 0.0f) {
            soulLance(chamber);
            soulLanceCooldown = 10.0f;
        }
    }

    Enemy::updateState(dt, chamber);
}

int BossMalachar::getCurrentPhase() const {
    return currentPhase;
}

void BossMalachar::transitionPhase(int phase) {
    currentPhase = phase;
    std::cout << "[BossMalachar] Transitioning to Phase " << currentPhase << "!\n";
}

void BossMalachar::platformSunder(Chamber& chamber) {
    std::cout << "[BossMalachar] Casting Platform Sunder!\n";
    BossChamber* bossChamber = dynamic_cast<BossChamber*>(&chamber);
    if (bossChamber) {
        // Target player's current platform for sunder
        sf::Vector2f playerPos = getPlayer().getPosition();
        bossChamber->sunderPlatformAt(playerPos);
    }
}

void BossMalachar::soulLance(Chamber& chamber) {
    std::cout << "[BossMalachar] Casting Soul Lance!\n";
    // Soul Lance deals 30 damage to player
    getPlayer().takeDamage(30.0f);
}

void BossMalachar::onDeath(Chamber* chamber) {
    std::cout << "[BossMalachar] Malachar has been defeated!\n";
    BossChamber* bossChamber = dynamic_cast<BossChamber*>(chamber);
    if (bossChamber) {
        bossChamber->onBossDefeated();
    }
}
