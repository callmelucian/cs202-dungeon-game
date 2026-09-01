#include "orb-projectile.hpp"
#include "../player.hpp"
#include "../effects/burned-effect.hpp"
#include "../../utils/math-utility.hpp"
#include <random>

OrbProjectile::OrbProjectile(sf::Vector2f startPos,
                             sf::Vector2f velocity,
                             float damage,
                             float radius,
                             float hitboxRadius,
                             float lifetime,
                             sf::Color fillColor,
                             sf::Color outlineColor,
                             float outlineThickness,
                             float burnChance)
    : Projectile(startPos, velocity, Math::length(velocity), 10000.0f, lifetime),
      damage(damage),
      radius(radius),
      hitboxRadius(hitboxRadius),
      fillColor(fillColor),
      outlineColor(outlineColor),
      outlineThickness(outlineThickness),
      burnChance(burnChance)
{
    this->velocity = velocity;
}

void OrbProjectile::setOnHitCallback(std::function<void(Player&)> callback) {
    onHitCallback = std::move(callback);
}

void OrbProjectile::update(float dt, Player& player) {
    if (!active) return;

    updateMotion(dt);

    if (active && player.isAlive()) {
        float dist = Math::distance(position, player.getPosition());
        if (dist < hitboxRadius) {
            player.takeDamage(damage);
            active = false;

            if (burnChance > 0.0f) {
                static std::mt19937 burnRng(std::random_device{}());
                std::uniform_real_distribution<float> burnDist(0.0f, 1.0f);
                if (burnDist(burnRng) < burnChance) {
                    player.applyStatusEffect(std::make_unique<BurnedEffect>(5.0f, 5.0f));
                }
            }

            if (onHitCallback) {
                onHitCallback(player);
            }
        }
    }
}

void OrbProjectile::draw(sf::RenderTarget& target) const {
    if (!active) return;

    sf::CircleShape orb(radius);
    orb.setOrigin({radius, radius});
    orb.setPosition(position);
    orb.setFillColor(fillColor);
    orb.setOutlineColor(outlineColor);
    orb.setOutlineThickness(outlineThickness);
    target.draw(orb);
}
