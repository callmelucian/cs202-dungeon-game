#ifndef CHOIR_HUSK_HPP
#define CHOIR_HUSK_HPP

#include "enemy.hpp"

enum class HuskVariant {
    STANDARD,
    CARRIER,
    GUARD
};

class ChoirHusk : public Enemy {
public:
    ChoirHusk(Player& player, HuskVariant variant = HuskVariant::STANDARD);
    
    void onDeath(Chamber* chamber = nullptr) override;
    void updateState(float dt, Chamber& chamber) override;

    // Call-and-response mechanics
    void startWindup();
    bool isWindingUp() const;
    
private:
    void triggerCallResponse(Chamber& chamber);

    HuskVariant variant;
    float windupTimer;
    bool windingUp;
    float callCooldown; // Prevents spamming calls constantly
};

#endif // CHOIR_HUSK_HPP