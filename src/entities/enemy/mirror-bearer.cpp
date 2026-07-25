#include "mirror-bearer.hpp"
#include "enemy-state.hpp"
#include "enemy-steering-strategy.hpp"
#include "../../chambers/chamber.hpp"
#include <iostream>

MirrorBearer::MirrorBearer(Player& player, MirrorVariant variant) 
    : Enemy("mirror_bearer", player), variant(variant) {
    
    if (variant == MirrorVariant::CARRIER) {
        baseStats.hp = baseStats.maxHp = 18.0f;
        fragmentDropCount = 3;
        setIsRealCarrier(true);
    } else { // GUARD (Decoy)
        baseStats.hp = baseStats.maxHp = 24.0f;
        fragmentDropCount = 0;
        setIsRealCarrier(false);
    }
    
    baseStats.speed = 6.5f;
    baseStats.damage = 10.0f; // Assumed since not specified

    setSteeringStrategy(std::make_unique<SeekStrategy>());
    changeState(std::make_unique<IdleState>());
}

void MirrorBearer::takeDamage(float rawAmount) {
    if (!isAlive()) return;

    float hpBefore = getHp();
    Enemy::takeDamage(rawAmount);

    // Real carrier: staggers visibly for 0.5s on any non-lethal hit
    if (variant == MirrorVariant::CARRIER && isAlive() && getHp() < hpBefore) {
        // Transition to StaggeredState, which will return to ChasingState after 0.5s
        changeState(std::make_unique<StaggeredState>(0.5f, std::make_unique<ChasingState>()));
    }
}

void MirrorBearer::onDeath(Chamber* chamber) {
    if (variant == MirrorVariant::CARRIER)
        std::cout << "MirrorBearer (Carrier) died, dropping 3 fragments!\n";
    else
        std::cout << "MirrorBearer (Decoy) shattered into smoke!\n";
}

bool MirrorBearer::canBeKnockedBack() const {
    // Decoy has no knockback reaction
    return variant == MirrorVariant::CARRIER;
}