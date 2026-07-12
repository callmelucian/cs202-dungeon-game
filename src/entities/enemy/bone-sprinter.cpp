#include "bone-sprinter.hpp"
#include "enemy-state.hpp"
#include "enemy-steering-strategy.hpp"
#include <iostream> 

BoneSprinter::BoneSprinter(Player& player, bool isCarrier) : Enemy("bone_sprinter", player), isCarrier(isCarrier) {
    if (isCarrier) {
        baseStats.hp = 14.0f;
        baseStats.maxHp = 14.0f;
        baseStats.speed = 9.0f;
        fragmentDropCount = 1;
    } else {
        baseStats.hp = 20.0f;
        baseStats.maxHp = 20.0f;
        baseStats.speed = 6.0f;
        fragmentDropCount = 0;
    }
    
    attackCooldown = 0.0f;

    setSteeringStrategy(std::make_unique<SeekStrategy>());

    changeState(std::make_unique<IdleState>());
}

void BoneSprinter::draw(sf::RenderWindow& window) const {
    // Wait for animation to be built
}

// Placeholder, will update later
void BoneSprinter::onDeath() {
    if (isCarrier) {
        std::cout << "BoneSprinter died and dropped " << fragmentDropCount << " Fragment(s)!\n";
    } else {
        std::cout << "BoneSprinter Blocker died.\n";
    }
}