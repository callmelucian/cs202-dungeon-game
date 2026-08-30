#ifndef BOSS_MALACHAR_HPP
#define BOSS_MALACHAR_HPP

#include "enemy.hpp"
#include "../effects/orb-projectile.hpp"
#include <vector>
#include <SFML/Graphics.hpp>

class BossChamber;

struct TrackingExplosionCircle {
    sf::Vector2f targetPos;
    float timer;
    float maxTimer;
    float radius;
    bool exploded;
};

/**
 * @brief BossMalachar represents the final boss entity of the Ashen Vault.
 * Implements a 4-phase combat cycle with Void Bolt, Summoning Burst, Platform Sunder,
 * Tracking Explosion Circles attack, and Echo interactions (Marrow Regen, Obsidian Blink, Resonance Core Burst, Foretell).
 */
class BossMalachar : public Enemy {
private:
    int currentPhase;
    float phaseTimer;
    float cycleTimer; // 12-second repeating cycle

    // Void Bolt Cycle state
    bool isVoidBoltCharging;
    float voidBoltTelegraphTimer;
    float voidBoltTelegraphMax;
    int voidBoltsRemaining;
    float voidBoltIntervalTimer;
    bool summoningBurstFired;

    // Phase 2-3 Modifiers
    float blinkTimer;
    float sunderCooldown;

    // Phase 4 Tracking Circle Attack
    float trackingAttackCooldown;
    int trackingCirclesRemaining;
    float trackingCircleIntervalTimer;
    std::vector<TrackingExplosionCircle> trackingCircles;

    // Hollow Bell & Foretell modifiers
    bool reflectWardActive;
    float reflectWardCooldown;
    bool foretellActive;

    // Resonance Core transition burst handling
    float pendingResonanceBurstTimer;
    bool pendingResonanceBurst;

    // Active projectiles
    std::vector<OrbProjectile> voidBolts;

    void applyRunStateModifiers();
    void resetCycle();
    void updateVoidBoltCycle(float dt, Chamber& chamber);
    void updateTrackingAttack(float dt, Chamber& chamber);
    void updateProjectiles(float dt);
    
    void applyMarrowRegen(float dt);
    void performBlink(Chamber& chamber);
    void resonanceCoreBurst();

public:
    BossMalachar(Player& player);
    virtual ~BossMalachar() = default;

    void update(float deltaTime) override;
    void updateState(float dt, Chamber& chamber) override;
    void draw(sf::RenderWindow& window) const override;
    sf::FloatRect getBounds() const override;
    void takeDamage(float rawAmount, bool isCritical = false) override;
    void onDeath(Chamber* chamber = nullptr) override;

    int getCurrentPhase() const;
    void transitionPhase(int phase);
    void platformSunder(Chamber& chamber);
    void startTrackingAttack(Chamber& chamber);
    void drawTrackingCircles(sf::RenderWindow& window) const;
    void cancelCharging();
    void clearProjectiles();

    bool canBeKnockedBack() const override { return false; }
    bool isShielded() const override { return !isFrozen(); }
};

#endif // BOSS_MALACHAR_HPP
