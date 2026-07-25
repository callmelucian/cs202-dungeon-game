#include "siege-wraith.hpp"
#include "enemy-state.hpp"
#include "enemy-steering-strategy.hpp"
#include "../../chambers/chamber.hpp"  // Fixed path (up two levels)
#include "../player.hpp"             // Added Player header
#include <iostream>

SiegeWraith::SiegeWraith(Player& player) : Enemy("siege_wraith", player) {
    baseStats.hp = 40.0f;
    baseStats.maxHp = 40.0f;
    baseStats.damage = 10.0f;
    baseStats.speed = 2.5f; // A bit slow to make it a fair "siege" enemy
    fragmentDropCount = 1;
    attackCooldown = 0.0f;

    setSteeringStrategy(std::make_unique<SeekStrategy>());
    changeState(std::make_unique<IdleState>());
}

void SiegeWraith::onDeath(Chamber* chamber) {
    std::cout << "SiegeWraith exploded!\n";
    if (chamber) explode(*chamber);
}

void SiegeWraith::explode(Chamber& chamber) {
    sf::Vector2f myPos = getPosition();
    float explosionRadius = 3.0f;
    float explosionDamage = 15.0f;

    Player& player = getPlayer();
    if (CollisionSolver::circleIntersect(myPos, explosionRadius, player.getPosition(), 0.0f))
        player.takeDamage(explosionDamage);

    for (Enemy* e : chamber.getEnemiesRaw()) {
        if (e != this && e->isAlive()) {
            // Check radius using circleIntersect (with 0.0f for target radius treating them as points)
            if (CollisionSolver::circleIntersect(myPos, explosionRadius, e->getPosition(), 0.0f)) {
                e->takeDamage(explosionDamage);
                
                // Note: If this kills another Siege Wraith, it will naturally chain 
                // when the Chamber processes its death in the next update loop!
            }
        }
    }
}