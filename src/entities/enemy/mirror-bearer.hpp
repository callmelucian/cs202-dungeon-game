#ifndef MIRROR_BEARER_HPP
#define MIRROR_BEARER_HPP

#include "enemy.hpp"

enum class MirrorVariant {
    CARRIER,
    GUARD // Decoy
};

class MirrorBearer : public Enemy {
public:
    MirrorBearer(Player& player, MirrorVariant variant = MirrorVariant::GUARD);
    
    void takeDamage(float rawAmount) override;
    void onDeath(Chamber* chamber = nullptr) override;
    bool canBeKnockedBack() const override;

private:
    MirrorVariant variant;
};

#endif // MIRROR_BEARER_HPP