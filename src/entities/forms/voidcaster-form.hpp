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

class VoidcasterLanceState : public SpecialAbilityState {
public:
    VoidcasterLanceState(PlayerCombatState* inner);
    StatModifier getStatModifier() const override;
    const std::string& getVisualKey() override;
    float modifyOutgoingDamage(float baseAmount) override;
    void onAttack(Player& player, sf::Vector2f targetDir, Chamber& chamber) override;
};

class VoidcasterDetonationFieldState : public SpecialAbilityState {
public:
    VoidcasterDetonationFieldState(PlayerCombatState* inner);
    StatModifier getStatModifier() const override;
    const std::string& getVisualKey() override;
    void onEnemyHit(Player& player, Enemy* enemy, bool lethal, Chamber& chamber) override;
    void draw(const Player& player, sf::RenderWindow& window) const override;

private:
    bool isExplosionActive = false;
};

#endif // VOIDCASTER_FORM_HPP
