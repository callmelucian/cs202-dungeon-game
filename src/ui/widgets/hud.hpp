#ifndef HUD_HPP
#define HUD_HPP

#include "../base/component.hpp"
#include "../../economy/echo.hpp"
#include "../../entities/player.hpp"
#include "../../core/enums.hpp"
#include "../../global-settings/asset-manager.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace UI {

struct ActiveEffectInfo {
    std::string name;
    float timer;
};

class HUD : public Component, public EchoObserver {
public:
    HUD();
    virtual ~HUD() = default;

    // UI::Component interface
    void draw(sf::RenderTarget& target) const override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void onColorPaletteChanged(const ColorPalette& palette) override;
    void computeSize(sf::Vector2f availableSize) override;
    void setPosition(sf::Vector2f pos) override;

    // EchoObserver interface
    void onEchoPowerChanged(float power) override;

    // Main sync methods called every frame from GameplayState
    void updatePlayerState(const Player& player);
    void updateChamberInfo(int level, int chamber, const std::string& title, float elapsedTime);

    // Enable / disable top Echo Power bar
    void setHasEcho(bool active);

private:
    // Health state & smooth lerp
    float currentHp;
    float maxHp;
    float displayedHpRatio;
    float ghostHpRatio;
    float ghostLagTimer;

    // Active Form & Momentum state
    FormType activeForm;
    float currentMomentum;
    float displayedMomentum;

    // Switch cooldown state
    float switchCooldownTimer;
    float maxSwitchCooldown;

    // Dash cooldown state
    float dashCooldownTimer;
    float maxDashCooldown;

    // Echo Power state & smooth lerp
    float echoPower;
    float displayedEchoRatio;
    bool hasEcho;

    // Chamber info & timer
    int chamberLevel;
    int chamberNumber;
    std::string chamberTitle;
    float elapsedChamberTime;

    // Active status effects
    std::vector<ActiveEffectInfo> activeEffects;

    // Smoothing & animation timers
    float lerpSpeed;
    float animTime;

    // Helper drawing routines
    void drawPlayerCluster(sf::RenderTarget& target) const;
    void drawStatusEffects(sf::RenderTarget& target) const;
    void drawChamberTimer(sf::RenderTarget& target) const;
    void drawEchoIntegrityBar(sf::RenderTarget& target) const;
};

} // namespace UI

#endif // HUD_HPP
