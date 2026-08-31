#ifndef VOIDCASTER_FORM_HPP
#define VOIDCASTER_FORM_HPP

#include "../player-form.hpp"
#include "../effects/arrow-projectile.hpp"

class VoidcasterForm : public PlayerForm {
public:
    VoidcasterForm();

    void update(Player& player, float dt) override;
    void attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) override;
    std::unique_ptr<SpecialAbilityState> createSpecialState(int abilityIndex) override;
    float getMomentumGainOnHit(float hpLost) const override;
    std::string getAttackAnimKey() const override;

    void setHitMode(ArrowHitMode mode) { hitMode = mode; }
    ArrowHitMode getHitMode() const { return hitMode; }

    void setArrowSpeed(float speed) { arrowSpeed = speed; }
    float getArrowSpeed() const { return arrowSpeed; }

private:
    ArrowHitMode hitMode = ArrowHitMode::PIERCING;
    float arrowSpeed = 900.0f;

    bool hasPendingArrow = false;
    float pendingTimer = 0.0f;
    sf::Vector2f pendingTargetDir;
    sf::Vector2f pendingTargetWorldPos;
    Chamber* pendingChamber = nullptr;

    void firePendingArrow(Player& player);
};

class VoidcasterTripleShotState : public SpecialAbilityState {
public:
    VoidcasterTripleShotState(PlayerCombatState* inner);
    StatModifier getStatModifier() const override;
    const std::string& getVisualKey() override;
};

class VoidcasterCosmicLaserState : public SpecialAbilityState {
public:
    VoidcasterCosmicLaserState(PlayerCombatState* inner);
    StatModifier getStatModifier() const override;
    const std::string& getVisualKey() override;
    float modifyCriticalRate(float baseRate) override;
};

#endif // VOIDCASTER_FORM_HPP
