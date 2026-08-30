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
                    float speed = 900.0f,
                    ArrowHitMode hitMode = ArrowHitMode::PIERCING);
    ~ArrowProjectile() override = default;

    void update(float dt, Chamber& chamber, Player& player);
    void draw(sf::RenderTarget& target) const override;
    void draw(sf::RenderWindow& window) const;

    ArrowHitMode getHitMode() const;

private:
    sf::Sprite sprite;
    ArrowHitMode hitMode;
    std::unordered_set<Enemy*> hitEnemies;
};

#endif // ARROW_PROJECTILE_HPP
