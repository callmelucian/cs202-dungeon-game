#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "../character.hpp"
#include "../../ui/widgets/enemy-health-bar.hpp"
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
    bool isFacingRight = true;
    std::string facingString = "down";
    bool isRealCarrier;
    bool hitWall;

public:
    Enemy(const std::string& characterKey, Player& player);
    virtual ~Enemy();

    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) const override;
    void takeDamage(float rawAmount) override;
    void onWallCollision() override;

    virtual void updateState(float dt, Chamber& chamber);
    void changeState(std::unique_ptr<EnemyState> newState);

    void setSteeringStrategy(std::unique_ptr<EnemySteeringStrategy> strategy);
    EnemySteeringStrategy* getSteeringStrategy() const;
    
    virtual void onDeath(Chamber* chamber = nullptr) = 0;
    void addBonusFragments(int count);
    int getFragmentDropCount() const;

    void setHitWall(bool hit);
    bool getHitWall() const;

    Player& getPlayer() const;
    
    void setIsRealCarrier(bool real);
    bool getIsRealCarrier() const;

    UI::EnemyHealthBar* getEnemyHealthBar();
    const UI::EnemyHealthBar* getEnemyHealthBar() const;
    
    float getAttackCooldown() const { return attackCooldown; }
    void setAttackCooldown(float cd) { attackCooldown = cd; }

    virtual bool canBeKnockedBack() const { return true; }
};

#endif // ENEMY_HPP