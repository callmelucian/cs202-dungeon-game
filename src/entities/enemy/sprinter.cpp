#include "sprinter.hpp"
#include "enemy-state.hpp"
#include "enemy-steering-strategy.hpp"
#include "../player.hpp"

Sprinter::Sprinter(Player& player, bool isCarrier) : Enemy("sprinter", player), isCarrier(isCarrier) {
    baseStats.maxHp = 15.0f;
    baseStats.hp = 15.0f;
    baseStats.damage = 5.0f;
    baseStats.speed = 5.5f;

    setIsRealCarrier(isCarrier);

    if (isCarrier) {
        fragmentDropCount = 3;
        setSteeringStrategy(std::make_unique<EvadeStrategy>());
    } else {
        fragmentDropCount = 1;
        setSteeringStrategy(std::make_unique<SeekStrategy>());
    }

    changeState(std::make_unique<ChasingState>());
}

void Sprinter::onDeath(Chamber* chamber) {
}
