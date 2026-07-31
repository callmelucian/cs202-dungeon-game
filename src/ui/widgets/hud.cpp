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
static constexpr float HP_BAR_W     = 300.0f;
static constexpr float HP_BAR_H     = 26.0f;
static constexpr float MOM_BAR_W    = 260.0f;
static constexpr float MOM_BAR_H    = 20.0f;
static constexpr float MOM_SPACING  = 13.0f;
static constexpr float CD_BAR_W     = 260.0f;
static constexpr float CD_BAR_H     = 14.0f;
static constexpr float PANEL_W      = 330.0f;
static constexpr float PANEL_H      = 180.0f;
static constexpr float ECHO_BAR_W   = 400.0f;
static constexpr float ECHO_BAR_H   = 22.0f;
static constexpr float STATUS_ICON  = 20.0f;

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

// Helper: draw text
static void drawText(sf::RenderTarget& target, float x, float y, const std::string& str, unsigned int size = 14, sf::Color color = sf::Color::White) {
    const sf::Font& font = AssetManager::getInstance().getFont("regular");
    sf::Text text(font, str, size);
    text.setPosition({x, y});
    text.setFillColor(color);
    target.draw(text);
}

void HUD::drawMainPanel(sf::RenderTarget& target) const {
    float x = position.x + PAD;
    float y = position.y + PAD;

    // HP label
    drawText(target, x, y - 2.0f, "HP", 16, sf::Color(255, 100, 100));

    // Health bar
    float barX = x + 36.0f;
    float barW = HP_BAR_W - 36.0f;
    drawBar(target, barX, y, barW, HP_BAR_H,
            displayedHpRatio,
            sf::Color(60, 0, 0, 200),
            sf::Color(210, 50, 50));

    // Numeric HP overlay (e.g. "46/100")
    std::string hpStr = std::to_string(static_cast<int>(std::round(currentHp)))
                      + "/" + std::to_string(static_cast<int>(std::round(maxHp)));
    drawText(target, barX + 4.0f, y + 2.0f, hpStr, 12, sf::Color(255, 220, 220));

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
    float topY = position.y + PAD + HP_BAR_H + 24.0f;

    auto rowColor = [&](FormType form) -> std::pair<sf::Color, sf::Color> {
        if (form == FormType::WRAITHBLADE)
            return {sf::Color(60, 0, 0, 140),  sf::Color(220, 60, 60)};
        if (form == FormType::VOIDCASTER)
            return {sf::Color(40, 0, 60, 140), sf::Color(180, 60, 220)};
        // IRONSHELL
        return   {sf::Color(50, 40, 0, 140),   sf::Color(220, 200, 50)};
    };

    float ratio = 0.0f;
    float rawMom = 0.0f;
    if (activeForm == FormType::WRAITHBLADE) { ratio = displayedWraithblade; rawMom = wraithbladeMomentum; }
    else if (activeForm == FormType::VOIDCASTER) { ratio = displayedVoidcaster; rawMom = voidcasterMomentum; }
    else if (activeForm == FormType::IRONSHELL) { ratio = displayedIronshell; rawMom = ironshellMomentum; }

    auto [bg, fg] = rowColor(activeForm);

    drawText(target, x, topY - 16.0f, "Momentum", 13, sf::Color(200, 200, 200));
    drawBar(target, x, topY, MOM_BAR_W, MOM_BAR_H, ratio, bg, fg);

    // Numeric momentum overlay (e.g. "46/100")
    std::string momStr = std::to_string(static_cast<int>(std::round(rawMom))) + "/100";
    drawText(target, x + 4.0f, topY + 1.0f, momStr, 11, sf::Color(240, 240, 240));
}

void HUD::drawCooldownBar(sf::RenderTarget& target) const {
    if (switchCooldownTimer <= 0.0f) return;

    float x = position.x + PAD;
    float y = position.y + PAD + HP_BAR_H + 24.0f + MOM_BAR_H + 28.0f;

    drawText(target, x, y - 16.0f, "Switch CD", 13, sf::Color(150, 200, 220));

    float ratio = std::clamp(switchCooldownTimer / maxSwitchCooldown, 0.0f, 1.0f);
    drawBar(target, x, y, CD_BAR_W, CD_BAR_H, ratio,
            sf::Color(20, 40, 50, 160),
            sf::Color(80, 200, 220));

    // Numeric cooldown overlay (e.g. "2.4s")
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1fs", switchCooldownTimer);
    drawText(target, x + 4.0f, y + 0.0f, buf, 10, sf::Color(200, 240, 255));
}

void HUD::drawStatusEffects(sf::RenderTarget& target) const {
    if (activeEffects.empty()) return;

    float x = position.x + PAD;
    float y = position.y + PAD + HP_BAR_H + 24.0f + MOM_BAR_H + 28.0f + CD_BAR_H + 16.0f;

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
    float y = position.y + 30.0f;

    drawText(target, x, y - 18.0f, "Echo Power", 15, sf::Color(0, 220, 180));

    drawBar(target, x, y, ECHO_BAR_W, ECHO_BAR_H,
            displayedEchoRatio,
            sf::Color(0, 40, 40, 200),
            sf::Color(0, 220, 180));

    // Numeric echo power overlay (e.g. "78/100")
    std::string echoStr = std::to_string(static_cast<int>(std::round(echoPower))) + "/100";
    drawText(target, x + 4.0f, y + 2.0f, echoStr, 12, sf::Color(200, 255, 245));

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

