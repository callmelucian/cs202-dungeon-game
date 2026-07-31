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

    // Main sync method called every frame from GameplayState
    void updatePlayerState(const Player& player);

    // Enable / disable top Echo Power bar
    void setHasEcho(bool active);

private:
    // Health state & smooth lerp
    float currentHp;
    float maxHp;
    float displayedHpRatio;

    // Form state
    FormType activeForm;

    // 3 Persistent Momentum values & smooth lerps
    float wraithbladeMomentum;
    float voidcasterMomentum;
    float ironshellMomentum;

    float displayedWraithblade;
    float displayedVoidcaster;
    float displayedIronshell;

    // Switch cooldown state
    float switchCooldownTimer;
    float maxSwitchCooldown;

    // Echo Power state & smooth lerp
    float echoPower;
    float displayedEchoRatio;
    bool hasEcho;

    // Active status effects
    std::vector<ActiveEffectInfo> activeEffects;

    // Smoothing speed
    float lerpSpeed;

    // Helper drawing routines
    void drawMainPanel(sf::RenderTarget& target) const;
    void drawMomentumMeters(sf::RenderTarget& target) const;
    void drawCooldownBar(sf::RenderTarget& target) const;
    void drawStatusEffects(sf::RenderTarget& target) const;
    void drawEchoPowerBar(sf::RenderTarget& target) const;
};

} // namespace UI

#endif // HUD_HPP
