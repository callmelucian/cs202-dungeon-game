#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "../character.hpp"
#include <memory>

class Player;
class Chamber;
class EnemyState;
class EnemySteeringStrategy;

class Enemy : public Character {
protected:
    Player& playerRef;
    std::unique_ptr<EnemyState> currentState;
    std::unique_ptr<EnemySteeringStrategy> steeringStrategy;
    float attackCooldown;
    int fragmentDropCount;
    bool isFacingRight;
    bool isRealCarrier;

public:
    Enemy(const std::string& characterKey, Player& player);
    virtual ~Enemy();

    void update(float deltaTime) override;

    virtual void updateState(float dt, Chamber& chamber);
    void changeState(std::unique_ptr<EnemyState> newState);

    void setSteeringStrategy(std::unique_ptr<EnemySteeringStrategy> strategy);
    EnemySteeringStrategy* getSteeringStrategy() const;
    
    virtual void onDeath() = 0;

    Player& getPlayer() const;
    
    void setIsRealCarrier(bool real);
    bool getIsRealCarrier() const;
};

#endif // ENEMY_HPP