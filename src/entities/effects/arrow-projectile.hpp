#ifndef ARROW_PROJECTILE_HPP
#define ARROW_PROJECTILE_HPP

#include "projectile.hpp"
#include <unordered_set>

class Chamber;
class Player;
class Enemy;

enum class ArrowHitMode {
    PIERCING,
    SINGLE_TARGET
};

class ArrowProjectile : public Projectile {
public:
    ArrowProjectile(sf::Vector2f startPos,
                    sf::Vector2f direction,
                    float maxDistance,
                    float speed = 1000.0f,
                    ArrowHitMode hitMode = ArrowHitMode::SINGLE_TARGET,
                    bool isRedLaser = false);
    ~ArrowProjectile() override = default;

    void update(float dt, Chamber& chamber, Player& player);
    void draw(sf::RenderTarget& target) const override;
    void draw(sf::RenderWindow& window) const;

    ArrowHitMode getHitMode() const;
    bool getIsRedLaser() const { return isRedLaser; }

private:
    sf::Sprite sprite;
    ArrowHitMode hitMode;
    bool isRedLaser;
    std::unordered_set<Enemy*> hitEnemies;
};

#endif // ARROW_PROJECTILE_HPP
