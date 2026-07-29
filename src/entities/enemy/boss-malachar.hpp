#ifndef BOSS_MALACHAR_HPP
#define BOSS_MALACHAR_HPP

#include "enemy.hpp"

class BossChamber;

/**
 * @brief BossMalachar represents the final boss entity of the Ashen Vault.
 * Delegates chamber/platform state management to BossChamber while running 
 * multi-phase AI combat logic, sunder attacks, and soul lances.
 */
class BossMalachar : public Enemy {
private:
    int currentPhase;
    float phaseTimer;
    float attackTimer;
    float sunderCooldown;
    float soulLanceCooldown;

public:
    BossMalachar(Player& player);
    virtual ~BossMalachar() = default;

    void update(float deltaTime) override;
    void updateState(float dt, Chamber& chamber) override;
    void onDeath(Chamber* chamber = nullptr) override;

    int getCurrentPhase() const;
    void transitionPhase(int phase);
    void platformSunder(Chamber& chamber);
    void soulLance(Chamber& chamber);

    bool canBeKnockedBack() const override { return false; }
};

#endif // BOSS_MALACHAR_HPP
