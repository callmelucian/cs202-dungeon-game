#ifndef BOSS_CHAMBER_HPP
#define BOSS_CHAMBER_HPP

#include "chamber.hpp"
#include "../entities/enemy/boss-malachar.hpp"
#include <vector>
#include <memory>

/**
 * @brief Platform represents a floating platform in the BossChamber.
 */
struct VoidRuptureZone {
    sf::Vector2f center; // Center position in world coordinates
    float radius;        // Radius in grid units
    bool isWarning;      // Telegraphing sunder attack (3s warning)
    float warningTimer;  // Telegraph countdown timer (seconds)
    bool isActive;       // Active void rupture zone (12s duration)
    float activeTimer;   // Active countdown timer (seconds)
};

enum class PhaseTransitionStage {
    NONE,
    FREEZE_AND_CLEAR,
    ZOOM_OUT,
    FADE_LERP_ISLANDS,
    ZOOM_IN
};

/**
 * @brief BossChamber wraps BossMalachar and owns the arena state,
 * phase state transitions, and Void Sunder ground rupture hazards.
 */
class BossChamber : public Chamber {
private:
    std::unique_ptr<BossMalachar> boss;
    int currentPhase;
    std::vector<VoidRuptureZone> ruptureZones;
    float sunderDamageTimer = 0.0f; // Accumulator for 1.0s periodic damage tick

    // Phase Transition Sequence Control
    PhaseTransitionStage transitionStage = PhaseTransitionStage::NONE;
    float transitionTimer = 0.0f;
    float fadeAlpha = 0.0f;
    int pendingNewPhase = 1;

    void updateRuptureZones(float dt);
    void drawRuptureZones(sf::RenderWindow& window);
    void updatePhaseTransitionSequence(float dt);

public:
    BossChamber(Player& player);
    virtual ~BossChamber() = default;

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void drawBackground(sf::RenderWindow& window) override;
    std::vector<sf::FloatRect> getObstaclesFor(const Character* character) const override;
    int processPlayerAttack(const Hitbox& hitbox) override;
    std::vector<Enemy*> getEnemiesRaw() const override;
    void onEnemyHit(Enemy* enemy, bool lethal) override;
    void freezeAllEnemies(float duration) override;
    void setGrids2D5(const std::vector<std::vector<std::string>>& tGrid, const std::vector<std::vector<int>>& lGrid) override;

    int getCurrentPhase() const;
    void setCurrentPhase(int phase);
    void triggerPhaseTransition(int newPhase);

    bool isPhaseTransitioning() const { return transitionStage != PhaseTransitionStage::NONE; }
    PhaseTransitionStage getTransitionStage() const { return transitionStage; }
    float getFadeAlpha() const { return fadeAlpha; }

    void sunderPlatformAt(const sf::Vector2f& pos);

    const std::vector<VoidRuptureZone>& getRuptureZones() const { return ruptureZones; }
    BossMalachar* getBoss() const;
    void onBossDefeated();
};

#endif // BOSS_CHAMBER_HPP
