#ifndef DUMMY_ENEMY_HPP
#define DUMMY_ENEMY_HPP

#include "enemy.hpp"
#include <string>

// DummyEnemy: Harmless training target for tutorial chambers.
// Deals 0 damage to the player and can be stationary or wandering.
class DummyEnemy : public Enemy {
public:
    explicit DummyEnemy(Player& player, 
                        const std::string& textureKey = "soldier", 
                        float maxHp = 30.0f, 
                        float speed = 0.0f,
                        bool wander = false);
    virtual ~DummyEnemy() override = default;

    void onDeath(Chamber* chamber = nullptr) override;
};

#endif // DUMMY_ENEMY_HPP
