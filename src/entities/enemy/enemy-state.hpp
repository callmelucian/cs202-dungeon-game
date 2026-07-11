#ifndef ENEMY_STATE_HPP
#define ENEMY_STATE_HPP

#include "enemy.hpp"
#include <memory>

class Chamber;

class EnemyState {
public:
    virtual ~EnemyState() = default;

    virtual void onEnter(Enemy& enemy) = 0;
    virtual void onExit(Enemy& enemy) = 0;
    virtual void update(Enemy& enemy, float dt, Chamber& chamber) = 0;
};

class IdleState : public EnemyState {
public:
    void onEnter(Enemy& enemy) override;
    void onExit(Enemy& enemy) override;
    void update(Enemy& enemy, float dt, Chamber& chamber) override;
};

class ChasingState : public EnemyState {
public:
    void onEnter(Enemy& enemy) override;
    void onExit(Enemy& enemy) override;
    void update(Enemy& enemy, float dt, Chamber& chamber) override;
};

class AttackingState : public EnemyState {
public:
    void onEnter(Enemy& enemy) override;
    void onExit(Enemy& enemy) override;
    void update(Enemy& enemy, float dt, Chamber& chamber) override;
};

// for stagger, stun effect in week 4, 5
class StaggeredState : public EnemyState {
private:
    float duration;
    float elapsedTime;
    std::unique_ptr<EnemyState> previousState;

public:
    StaggeredState(float duration, std::unique_ptr<EnemyState> previous);
    
    void onEnter(Enemy& enemy) override;
    void onExit(Enemy& enemy) override;
    void update(Enemy& enemy, float dt, Chamber& chamber) override;
};

#endif // ENEMY_STATE_HPP
