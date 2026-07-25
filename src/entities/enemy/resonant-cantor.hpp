#ifndef RESONANT_CANTOR_HPP
#define RESONANT_CANTOR_HPP

#include "enemy.hpp"

class ResonantCantor : public Enemy {
public:
    ResonantCantor(Player& player);
    
    void onDeath(Chamber* chamber = nullptr) override;
    void updateState(float dt, Chamber& chamber) override;

private:
    float pulseTimer;
    
    void emitPulse();
};

#endif // RESONANT_CANTOR_HPP