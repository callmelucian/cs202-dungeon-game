#ifndef BOSS_CHAMBER_HPP
#define BOSS_CHAMBER_HPP

#include "chamber.hpp"
#include "../entities/enemy/boss-malachar.hpp"
#include <vector>
#include <memory>

/**
 * @brief Platform represents a floating platform in the BossChamber.
 */
struct Platform {
    int id;
    sf::Vector2f center; // Center position in world coordinates
    float radius;        // Radius in grid units
    bool isWarning;      // Telegraphing sunder attack
    float sunderTimer;   // Telegraph countdown timer (seconds)
    bool isSundered;     // Platform collapsed into void
    float recoveryTimer; // Timer before platform reforms (if applicable)
};

enum class PhaseTransitionStage {
    NONE,
    FREEZE_AND_CLEAR,
    ZOOM_OUT,
    FADE_LERP_ISLANDS,
    ZOOM_IN
};

/**
 * @brief BossChamber wraps BossMalachar and owns the platform layout,
 * phase state transitions, and arena environmental mechanics.
 */
class BossChamber : public Chamber {
private:
    std::unique_ptr<BossMalachar> boss;
    int currentPhase;
    std::vector<Platform> platforms;
    float shrinkRate;   // Radius reduction in units per second (0.1 in Phase 4)
    float minRadius;    // Floor radius limit in units (1.5 in Phase 4)

    // Phase Transition Sequence Control
    PhaseTransitionStage transitionStage = PhaseTransitionStage::NONE;
    float transitionTimer = 0.0f;
    float fadeAlpha = 0.0f;
    int pendingNewPhase = 1;

    void initPlatforms();
    void updatePlatforms(float dt);
    void drawPlatforms(sf::RenderWindow& window);
    void updatePhaseTransitionSequence(float dt);
    void applyMapLayoutForPhase(int phase);

public:
    BossChamber(Player& player);
    virtual ~BossChamber() = default;

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    int processPlayerAttack(const Hitbox& hitbox) override;
    void setGrids2D5(const std::vector<std::vector<std::string>>& tGrid, const std::vector<std::vector<int>>& lGrid) override;

    int getCurrentPhase() const;
    void setCurrentPhase(int phase);
    void triggerPhaseTransition(int newPhase);

    bool isPhaseTransitioning() const { return transitionStage != PhaseTransitionStage::NONE; }
    PhaseTransitionStage getTransitionStage() const { return transitionStage; }
    float getFadeAlpha() const { return fadeAlpha; }

    void sunderPlatformAt(const sf::Vector2f& pos);
    void sunderPlatform(int index);

    const std::vector<Platform>& getPlatforms() const;
    BossMalachar* getBoss() const;
    void onBossDefeated();
};

#endif // BOSS_CHAMBER_HPP
