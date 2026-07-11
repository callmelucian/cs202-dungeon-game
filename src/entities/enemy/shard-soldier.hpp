#ifndef SHARD_SOLDIER_HPP
#define SHARD_SOLDIER_HPP

#include "enemy.hpp"

class ShardSoldier : public Enemy {
private:
    bool selfHealActive;

public:
    ShardSoldier(Player& player);
    
    void draw(sf::RenderWindow& window) const override;
    void onDeath() override;
    void update(float deltaTime) override; 
    
    void applySelfHeal(float dt);
    void setSelfHealActive(bool active);
    bool isSelfHealActive() const;

};

#endif // SHARD_SOLDIER_HPP