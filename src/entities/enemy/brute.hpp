#ifndef BRUTE_HPP
#define BRUTE_HPP

#include "enemy.hpp"
#include "../effects/orb-projectile.hpp"
#include <vector>

class Brute : public Enemy {
private:
    float shootTimer;
    float shootInterval;
    float chargeDuration;
    std::vector<OrbProjectile> projectiles;

public:
    Brute(Player& player);
    ~Brute() override = default;

    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) const override;
    void onDeath(Chamber* chamber = nullptr) override;

    bool isCharging() const { return isAlive() && !isFrozen() && shootTimer <= chargeDuration; }
};

#endif // BRUTE_HPP
