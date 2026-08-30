#ifndef ORB_PROJECTILE_HPP
#define ORB_PROJECTILE_HPP

#include "projectile.hpp"
#include <functional>

class Player;

class OrbProjectile : public Projectile {
public:
    OrbProjectile(sf::Vector2f startPos,
                  sf::Vector2f velocity,
                  float damage,
                  float radius = 5.0f,
                  float hitboxRadius = 12.0f,
                  float lifetime = 4.0f,
                  sf::Color fillColor = sf::Color(240, 120, 30, 245),
                  sf::Color outlineColor = sf::Color(255, 200, 100, 255),
                  float outlineThickness = 1.5f,
                  float burnChance = 0.0f);
    ~OrbProjectile() override = default;

    void update(float dt, Player& player);
    void draw(sf::RenderTarget& target) const override;

    float getDamage() const { return damage; }
    void setOnHitCallback(std::function<void(Player&)> callback);

private:
    float damage;
    float radius;
    float hitboxRadius;
    sf::Color fillColor;
    sf::Color outlineColor;
    float outlineThickness;
    float burnChance;
    std::function<void(Player&)> onHitCallback;
};

#endif // ORB_PROJECTILE_HPP
