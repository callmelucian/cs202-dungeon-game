#include "dummy-enemy.hpp"
#include "enemy-state.hpp"
#include "random-wander-strategy.hpp"
#include "../player.hpp"
#include "../../ui/graphics/particle-system.hpp"
#include "../../global-settings/sound-manager.hpp"

DummyEnemy::DummyEnemy(Player& player, 
                       const std::string& textureKey, 
                       float maxHp, 
                       float speed,
                       bool wander)
    : Enemy(textureKey, player)
{
    baseStats.maxHp = maxHp;
    baseStats.hp = maxHp;
    baseStats.damage = 0.0f; // Harmless in tutorial
    baseStats.speed = speed;
    fragmentDropCount = 0;

    // Distinct warm golden/amber tint for training dummies
    setTint(sf::Color(255, 225, 140));

    if (wander && speed > 0.0f) {
        setSteeringStrategy(std::make_unique<RandomWanderStrategy>(1.5f, 90.0f));
    } else {
        setSteeringStrategy(nullptr);
        setVelocity({0.0f, 0.0f});
    }

    changeState(std::make_unique<ChasingState>());
}

void DummyEnemy::onDeath(Chamber* /*chamber*/) {
    SoundManager::getInstance().playSound("enemy-hit");
    ParticleSystem::getInstance().emitBurst(getPosition(), 20, sf::Color(255, 220, 100, 220), 40.0f, 120.0f, 0.2f, 0.6f, 3.5f);
}
