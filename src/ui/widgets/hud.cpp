#include "hud.hpp"
#include "../../utils/math-utility.hpp"
#include "../../global-settings/setting-manager.hpp"
#include "../../entities/effects/status-effect.hpp"
#include <cmath>
#include <algorithm>

namespace UI {

HUD::HUD()
    : currentHp(0.0f),
      maxHp(0.0f),
      displayedHpRatio(1.0f),
      activeForm(FormType::WRAITHBLADE),
      wraithbladeMomentum(0.0f),
      voidcasterMomentum(0.0f),
      ironshellMomentum(0.0f),
      displayedWraithblade(0.0f),
      displayedVoidcaster(0.0f),
      displayedIronshell(0.0f),
      switchCooldownTimer(0.0f),
      maxSwitchCooldown(SWITCH_COOLDOWN_DURATION),
      echoPower(100.0f),
      displayedEchoRatio(1.0f),
      hasEcho(false),
      lerpSpeed(HUD_LERP_SPEED)
{
    SettingManager& settings = SettingManager::getInstance();
    fixedWidth  = static_cast<float>(settings.getWindowWidth());
    fixedHeight = static_cast<float>(settings.getWindowHeight());
    modeX = SizeMode::Fixed;
    modeY = SizeMode::Fixed;
    size     = sf::Vector2f(fixedWidth, fixedHeight);
    position = sf::Vector2f(0.0f, 0.0f);
}

void HUD::updatePlayerState(const Player& player) {
    // --- Health ---
    Stats stats = player.getEffectiveStats();
    currentHp = stats.hp;
    maxHp     = stats.maxHp;

    activeForm = player.getActiveFormType();

    wraithbladeMomentum = player.getMomentum(FormType::WRAITHBLADE);
    voidcasterMomentum  = player.getMomentum(FormType::VOIDCASTER);
    ironshellMomentum   = player.getMomentum(FormType::IRONSHELL);
    switchCooldownTimer = player.getSwitchCooldownTimer();
    activeEffects.clear();
    for (const auto& effect : player.getStatusEffects()) {
        ActiveEffectInfo info;
        info.name  = effect->getName();
        info.timer = effect->getTimer();
        activeEffects.push_back(info);
    }
}

void HUD::update(float dt) {
    if (dt <= 0.0f) return;
    float t = 1.0f - std::exp(-lerpSpeed * dt);

    // Smooth health bar
    float targetHpRatio = (maxHp > 0.0f) ? (currentHp / maxHp) : 0.0f;
    displayedHpRatio = Math::lerp(displayedHpRatio, targetHpRatio, t);

    // Smooth 3 momentum bars (ratio 0..1)
    displayedWraithblade = Math::lerp(displayedWraithblade, wraithbladeMomentum / MAX_MOMENTUM, t);
    displayedVoidcaster  = Math::lerp(displayedVoidcaster,  voidcasterMomentum  / MAX_MOMENTUM, t);
    displayedIronshell   = Math::lerp(displayedIronshell,   ironshellMomentum   / MAX_MOMENTUM, t);

    // Smooth Echo Power bar (ratio 0..1)
    float targetEchoRatio = echoPower / 100.0f;
    displayedEchoRatio = Math::lerp(displayedEchoRatio, targetEchoRatio, t);
}

void HUD::handleEvent(const sf::Event& /*event*/) {
    // HUD is non-interactive; no event handling needed
}

void HUD::onColorPaletteChanged(const ColorPalette& /*palette*/) {
    // Reserved for future theme support
}

void HUD::computeSize(sf::Vector2f /*availableSize*/) {
    // HUD always fills the full window — size is set from SettingManager in constructor
    size = sf::Vector2f(fixedWidth, fixedHeight);
}

void HUD::setPosition(sf::Vector2f pos) {
    position = pos;
}

void HUD::onEchoPowerChanged(float power) {
    echoPower = power;
}

void HUD::setHasEcho(bool active) {
    hasEcho = active;
}

void HUD::draw(sf::RenderTarget& target) const {
    drawMainPanel(target);
    drawMomentumMeters(target);
    drawCooldownBar(target);
    drawStatusEffects(target);
    if (hasEcho) drawEchoPowerBar(target);
}

// ---- Layout constants (relative to HUD position) ----
static constexpr float PAD          = 20.0f;
static constexpr float HP_BAR_W     = 200.0f;
static constexpr float HP_BAR_H     = 18.0f;
static constexpr float MOM_BAR_W    = 160.0f;
static constexpr float MOM_BAR_H    = 8.0f;
static constexpr float MOM_SPACING  = 13.0f;
static constexpr float CD_BAR_W     = 160.0f;
static constexpr float CD_BAR_H     = 5.0f;
static constexpr float PANEL_W      = 210.0f;
static constexpr float PANEL_H      = 130.0f;
static constexpr float ECHO_BAR_W   = 280.0f;
static constexpr float ECHO_BAR_H   = 12.0f;
static constexpr float STATUS_ICON  = 14.0f;

// Helper: draw a background + foreground filled bar at (x,y)
static void drawBar(sf::RenderTarget& target,
                    float x, float y, float w, float h,
                    float ratio,
                    sf::Color bgCol, sf::Color fgCol) {
    // Background track
    sf::RectangleShape bg({w, h});
    bg.setPosition({x, y});
    bg.setFillColor(bgCol);
    target.draw(bg);
    // Foreground fill — clamp to at least 1px to avoid zero-size assertion
    float fillW = std::max(1.0f, w * std::clamp(ratio, 0.0f, 1.0f));
    sf::RectangleShape fg({fillW, h});
    fg.setPosition({x, y});
    fg.setFillColor(fgCol);
    target.draw(fg);
}

void HUD::drawMainPanel(sf::RenderTarget& target) const {
    float x = position.x + PAD;
    float y = position.y + PAD;

    // Semi-transparent panel background
    sf::RectangleShape panel({PANEL_W, PANEL_H});
    panel.setPosition({x - 8.0f, y - 8.0f});
    panel.setFillColor(sf::Color(10, 10, 20, 180));
    target.draw(panel);

    // Health bar
    drawBar(target, x, y, HP_BAR_W, HP_BAR_H,
            displayedHpRatio,
            sf::Color(60, 0, 0, 200),
            sf::Color(210, 50, 50));

    // Form colour chip next to health bar
    sf::Color formColor = sf::Color::White;
    if (activeForm == FormType::WRAITHBLADE) formColor = sf::Color(220, 60, 60);
    else if (activeForm == FormType::VOIDCASTER) formColor = sf::Color(180, 60, 220);
    else if (activeForm == FormType::IRONSHELL)  formColor = sf::Color(220, 200, 50);

    sf::RectangleShape chip({HP_BAR_H, HP_BAR_H});
    chip.setPosition({x + HP_BAR_W + 6.0f, y});
    chip.setFillColor(formColor);
    target.draw(chip);
}

void HUD::drawMomentumMeters(sf::RenderTarget& target) const {
    float x   = position.x + PAD;
    float topY = position.y + PAD + HP_BAR_H + 10.0f;

    auto rowColor = [&](FormType form) -> std::pair<sf::Color, sf::Color> {
        bool active = (activeForm == form);
        if (form == FormType::WRAITHBLADE)
            return {sf::Color(60, 0, 0, 140),  active ? sf::Color(220, 60, 60)  : sf::Color(130, 30, 30)};
        if (form == FormType::VOIDCASTER)
            return {sf::Color(40, 0, 60, 140),  active ? sf::Color(180, 60, 220) : sf::Color(100, 30, 140)};
        // IRONSHELL
        return   {sf::Color(50, 40, 0, 140),   active ? sf::Color(220, 200, 50) : sf::Color(130, 120, 30)};
    };

    struct Row { float ratio; FormType form; };
    Row rows[3] = {
        {displayedWraithblade, FormType::WRAITHBLADE},
        {displayedVoidcaster,  FormType::VOIDCASTER},
        {displayedIronshell,   FormType::IRONSHELL},
    };

    for (int i = 0; i < 3; ++i) {
        auto [bg, fg] = rowColor(rows[i].form);
        float rowY = topY + i * MOM_SPACING;

        // Active form row is slightly wider/brighter
        float w = (activeForm == rows[i].form) ? MOM_BAR_W : MOM_BAR_W - 10.0f;
        float h = (activeForm == rows[i].form) ? MOM_BAR_H + 1.0f : MOM_BAR_H - 1.0f;

        drawBar(target, x, rowY, w, h, rows[i].ratio, bg, fg);
    }
}

void HUD::drawCooldownBar(sf::RenderTarget& target) const {
    if (switchCooldownTimer <= 0.0f) return;

    float x = position.x + PAD;
    float y = position.y + PAD + HP_BAR_H + 3 * MOM_SPACING + 14.0f;

    float ratio = std::clamp(switchCooldownTimer / maxSwitchCooldown, 0.0f, 1.0f);
    drawBar(target, x, y, CD_BAR_W, CD_BAR_H, ratio,
            sf::Color(20, 40, 50, 160),
            sf::Color(80, 200, 220));
}

void HUD::drawStatusEffects(sf::RenderTarget& target) const {
    if (activeEffects.empty()) return;

    float x = position.x + PAD;
    float y = position.y + PAD + HP_BAR_H + 3 * MOM_SPACING + 14.0f + CD_BAR_H + 8.0f;

    for (size_t i = 0; i < activeEffects.size(); ++i) {
        sf::Color col = sf::Color::White;
        if (activeEffects[i].name == "Burned")    col = sf::Color(255, 120, 30);
        else if (activeEffects[i].name == "Slowed")    col = sf::Color(80, 120, 255);
        else if (activeEffects[i].name == "Paralyzed") col = sf::Color(220, 220, 40);

        sf::RectangleShape icon({STATUS_ICON, STATUS_ICON});
        icon.setPosition({x + i * (STATUS_ICON + 4.0f), y});
        icon.setFillColor(col);
        // Fade icon as the effect expires (timer / 10s)
        uint8_t alpha = static_cast<uint8_t>(
            std::clamp(activeEffects[i].timer / 10.0f, 0.2f, 1.0f) * 255.0f);
        icon.setFillColor(sf::Color(col.r, col.g, col.b, alpha));
        target.draw(icon);
    }
}

void HUD::drawEchoPowerBar(sf::RenderTarget& target) const {
    // Centered at the top of the screen
    float x = position.x + (fixedWidth - ECHO_BAR_W) / 2.0f;
    float y = position.y + 8.0f;

    drawBar(target, x, y, ECHO_BAR_W, ECHO_BAR_H,
            displayedEchoRatio,
            sf::Color(0, 40, 40, 200),
            sf::Color(0, 220, 180));

    // Danger tint when Echo Power is critical (< 30%)
    if (displayedEchoRatio < 0.30f) {
        float fillW = std::max(1.0f, ECHO_BAR_W * displayedEchoRatio);
        sf::RectangleShape danger({fillW, ECHO_BAR_H});
        danger.setPosition({x, y});
        uint8_t alpha = static_cast<uint8_t>((1.0f - displayedEchoRatio / 0.30f) * 120.0f);
        danger.setFillColor(sf::Color(255, 60, 0, alpha));
        target.draw(danger);
    }
}

} // namespace UI

