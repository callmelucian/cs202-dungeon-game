#include "hud.hpp"
#include "../../utils/math-utility.hpp"
#include "../../global-settings/setting-manager.hpp"
#include "../../entities/effects/status-effect.hpp"
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cstdio>

namespace UI {

HUD::HUD()
    : currentHp(100.0f),
      maxHp(100.0f),
      displayedHpRatio(1.0f),
      ghostHpRatio(1.0f),
      ghostLagTimer(0.0f),
      activeForm(FormType::WRAITHBLADE),
      currentMomentum(0.0f),
      displayedMomentum(0.0f),
      switchCooldownTimer(0.0f),
      maxSwitchCooldown(SWITCH_COOLDOWN_DURATION),
      dashCooldownTimer(0.0f),
      maxDashCooldown(1.0f),
      echoPower(100.0f),
      displayedEchoRatio(1.0f),
      hasEcho(false),
      lerpSpeed(HUD_LERP_SPEED),
      animTime(0.0f)
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
    Stats stats = player.getEffectiveStats();
    currentHp = stats.hp;
    maxHp     = stats.maxHp;

    activeForm = player.getActiveFormType();
    currentMomentum = player.getMomentum(activeForm);
    switchCooldownTimer = player.getSwitchCooldownTimer();
    dashCooldownTimer   = player.getDashCooldownTimer();

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
    animTime += dt;
    float t = 1.0f - std::exp(-lerpSpeed * dt);

    // Smooth health bar
    float targetHpRatio = (maxHp > 0.0f) ? std::clamp(currentHp / maxHp, 0.0f, 1.0f) : 0.0f;
    displayedHpRatio = Math::lerp(displayedHpRatio, targetHpRatio, t);

    // Damage ghost bar lag
    if (ghostHpRatio < displayedHpRatio) {
        ghostHpRatio = displayedHpRatio;
        ghostLagTimer = 0.0f;
    } else if (ghostHpRatio > displayedHpRatio) {
        ghostLagTimer += dt;
        if (ghostLagTimer >= 0.35f) {
            float ghostT = 1.0f - std::exp(-3.5f * dt);
            ghostHpRatio = Math::lerp(ghostHpRatio, displayedHpRatio, ghostT);
        }
    }

    // Smooth current active momentum bar (0..1 ratio)
    displayedMomentum = Math::lerp(displayedMomentum, std::clamp(currentMomentum / MAX_MOMENTUM, 0.0f, 1.0f), t);

    // Smooth Echo Power bar (0..1 ratio)
    float targetEchoRatio = std::clamp(echoPower / 100.0f, 0.0f, 1.0f);
    displayedEchoRatio = Math::lerp(displayedEchoRatio, targetEchoRatio, t);
}

void HUD::handleEvent(const sf::Event& /*event*/) {
}

void HUD::onColorPaletteChanged(const ColorPalette& /*palette*/) {
}

void HUD::computeSize(sf::Vector2f /*availableSize*/) {
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
    drawPlayerCluster(target);
    drawStatusEffects(target);
    if (hasEcho) {
        drawEchoIntegrityBar(target);
    }
}

// =========================================================================
// Drawing Helper Functions
// =========================================================================

static void drawText(sf::RenderTarget& target,
                     const std::string& fontKey,
                     float x, float y,
                     const std::string& str,
                     unsigned int size = 14,
                     sf::Color color = sf::Color::White) {
    try {
        const sf::Font& font = AssetManager::getInstance().getFont(fontKey);
        sf::Text text(font, str, size);
        text.setPosition({x, y});
        text.setFillColor(color);
        target.draw(text);
    } catch (const std::exception&) {
        const sf::Font& fallback = AssetManager::getInstance().getFont("regular");
        sf::Text text(fallback, str, size);
        text.setPosition({x, y});
        text.setFillColor(color);
        target.draw(text);
    }
}

static void drawFramedBox(sf::RenderTarget& target,
                          float x, float y, float w, float h,
                          sf::Color bgColor,
                          sf::Color borderColor,
                          float borderThickness = 1.0f) {
    sf::RectangleShape box({w, h});
    box.setPosition({x, y});
    box.setFillColor(bgColor);
    box.setOutlineColor(borderColor);
    box.setOutlineThickness(borderThickness);
    target.draw(box);
}

static void drawGlossyBar(sf::RenderTarget& target,
                          float x, float y, float w, float h,
                          float fillRatio,
                          float ghostRatio,
                          sf::Color bgCol,
                          sf::Color fillCol,
                          sf::Color ghostCol,
                          sf::Color borderCol) {
    drawFramedBox(target, x, y, w, h, bgCol, borderCol, 1.0f);

    // Ghost Bar Fill (Damage Lag)
    if (ghostRatio > fillRatio && ghostRatio > 0.001f) {
        float ghostW = std::clamp(w * ghostRatio, 1.0f, w);
        sf::RectangleShape ghostRect({ghostW, h});
        ghostRect.setPosition({x, y});
        ghostRect.setFillColor(ghostCol);
        target.draw(ghostRect);
    }

    // Foreground Fill
    if (fillRatio > 0.001f) {
        float fillW = std::clamp(w * fillRatio, 1.0f, w);
        sf::RectangleShape fillRect({fillW, h});
        fillRect.setPosition({x, y});
        fillRect.setFillColor(fillCol);
        target.draw(fillRect);

        // Subtle gloss line
        if (h >= 6.0f) {
            sf::RectangleShape gloss({fillW, h * 0.35f});
            gloss.setPosition({x, y});
            gloss.setFillColor(sf::Color(255, 255, 255, 40));
            target.draw(gloss);
        }
    }
}

// =========================================================================
// 1. Compact Player Cluster (Health + Active Form Momentum + Dash)
// =========================================================================

void HUD::drawPlayerCluster(sf::RenderTarget& target) const {
    float startX = position.x + 20.0f;
    float startY = position.y + 20.0f;

    // --- Form Emblem Box (36x36) ---
    float emblemSize = 36.0f;
    sf::Color formColor = sf::Color(230, 60, 70);
    std::string formLetter = "W";

    if (activeForm == FormType::VOIDCASTER) {
        formColor = sf::Color(185, 80, 240);
        formLetter = "V";
    } else if (activeForm == FormType::IRONSHELL) {
        formColor = sf::Color(240, 195, 40);
        formLetter = "I";
    }

    drawFramedBox(target, startX, startY, emblemSize, emblemSize,
                  sf::Color(15, 18, 26, 230), formColor, 1.5f);

    drawText(target, "pixel-bold", startX + 9.0f, startY + 3.0f,
             formLetter, 22, sf::Color(255, 255, 255));

    // Form Swap Cooldown Overlay on Badge
    if (switchCooldownTimer > 0.0f) {
        sf::RectangleShape cdOverlay({emblemSize, emblemSize});
        cdOverlay.setPosition({startX, startY});
        cdOverlay.setFillColor(sf::Color(40, 10, 15, 180));
        target.draw(cdOverlay);

        char cdBuf[8];
        snprintf(cdBuf, sizeof(cdBuf), "%.1f", switchCooldownTimer);
        drawText(target, "bold", startX + 6.0f, startY + 10.0f, cdBuf, 11, sf::Color(255, 180, 180));
    }

    // --- Dash Mini-Bar (Below Emblem) ---
    float dashY = startY + emblemSize + 3.0f;
    float dashH = 4.0f;
    if (dashCooldownTimer <= 0.0f) {
        sf::RectangleShape dashBar({emblemSize, dashH});
        dashBar.setPosition({startX, dashY});
        dashBar.setFillColor(sf::Color(40, 220, 150));
        target.draw(dashBar);
    } else {
        float dashRatio = std::clamp(1.0f - dashCooldownTimer / maxDashCooldown, 0.0f, 1.0f);
        sf::RectangleShape dashTrack({emblemSize, dashH});
        dashTrack.setPosition({startX, dashY});
        dashTrack.setFillColor(sf::Color(20, 30, 40, 200));
        target.draw(dashTrack);

        sf::RectangleShape dashFill({emblemSize * dashRatio, dashH});
        dashFill.setPosition({startX, dashY});
        dashFill.setFillColor(sf::Color(30, 140, 180));
        target.draw(dashFill);
    }

    // --- Health Bar ---
    float barX = startX + emblemSize + 8.0f;
    float barY = startY;
    float barW = 230.0f;
    float barH = 18.0f;

    sf::Color hpBorder = sf::Color(65, 25, 30, 200);
    if (displayedHpRatio < 0.30f) {
        uint8_t pulseAlpha = static_cast<uint8_t>(140.0f + 100.0f * std::sin(animTime * 8.0f));
        hpBorder = sf::Color(255, 40, 40, pulseAlpha);
    }

    drawGlossyBar(target, barX, barY, barW, barH,
                  displayedHpRatio, ghostHpRatio,
                  sf::Color(25, 10, 15, 230),
                  sf::Color(220, 45, 55),
                  sf::Color(240, 140, 80, 200),
                  hpBorder);

    // HP Text Overlay: "HP  85 / 100"
    int intHp = static_cast<int>(std::round(currentHp));
    int intMaxHp = static_cast<int>(std::round(maxHp));
    std::string hpStr = "HP " + std::to_string(intHp) + " / " + std::to_string(intMaxHp);
    drawText(target, "bold", barX + 8.0f, barY + 2.0f, hpStr, 11, sf::Color(255, 245, 245));

    // --- Single Active Form Momentum Bar ---
    float momX = barX;
    float momY = startY + barH + 4.0f;
    float momW = barW;
    float momH = 14.0f;

    sf::Color momFill = sf::Color(225, 50, 60);
    if (activeForm == FormType::VOIDCASTER) momFill = sf::Color(175, 65, 235);
    else if (activeForm == FormType::IRONSHELL)  momFill = sf::Color(235, 185, 30);

    sf::Color momBorder = (currentMomentum >= 50.0f) ? sf::Color(255, 240, 100, 220) : sf::Color(40, 50, 68, 180);

    drawFramedBox(target, momX, momY, momW, momH,
                  sf::Color(15, 20, 30, 230), momBorder, 1.0f);

    if (displayedMomentum > 0.001f) {
        float fillW = std::clamp(momW * displayedMomentum, 1.0f, momW);
        sf::RectangleShape fillRect({fillW, momH});
        fillRect.setPosition({momX, momY});
        fillRect.setFillColor(momFill);
        target.draw(fillRect);

        // Subtle gloss
        sf::RectangleShape gloss({fillW, momH * 0.35f});
        gloss.setPosition({momX, momY});
        gloss.setFillColor(sf::Color(255, 255, 255, 35));
        target.draw(gloss);
    }

    // 50% Threshold Notch
    float notchX = momX + momW * 0.50f;
    sf::RectangleShape notch({1.0f, momH});
    notch.setPosition({notchX, momY});
    notch.setFillColor((currentMomentum >= 50.0f) ? sf::Color(255, 255, 255, 220) : sf::Color(0, 0, 0, 120));
    target.draw(notch);

    // Momentum Text Overlay
    int intMom = static_cast<int>(std::round(currentMomentum));
    std::string momStr = "MOM " + std::to_string(intMom) + " / 100";
    drawText(target, "regular", momX + 6.0f, momY + 1.0f, momStr, 10, sf::Color(240, 240, 240));

    if (currentMomentum >= 50.0f) {
        drawText(target, "bold", momX + momW - 108.0f, momY + 1.0f, "[Q] SPECIAL READY", 10, sf::Color(255, 240, 100));
    }
}

// =========================================================================
// 2. Compact Status Effects
// =========================================================================

void HUD::drawStatusEffects(sf::RenderTarget& target) const {
    if (activeEffects.empty()) return;

    float startX = position.x + 20.0f;
    float startY = position.y + 64.0f;
    float badgeW = 86.0f;
    float badgeH = 20.0f;
    float spacing = 6.0f;

    for (size_t i = 0; i < activeEffects.size(); ++i) {
        float x = startX + i * (badgeW + spacing);
        float y = startY;

        sf::Color badgeBg = sf::Color(20, 16, 25, 220);
        sf::Color badgeBorder = sf::Color(220, 120, 40, 220);
        sf::Color textColor = sf::Color(255, 200, 150);

        if (activeEffects[i].name == "Burned") {
            badgeBg = sf::Color(40, 15, 10, 230);
            badgeBorder = sf::Color(255, 100, 30, 220);
            textColor = sf::Color(255, 180, 120);
        } else if (activeEffects[i].name == "Slowed") {
            badgeBg = sf::Color(10, 20, 40, 230);
            badgeBorder = sf::Color(60, 160, 255, 220);
            textColor = sf::Color(160, 220, 255);
        } else if (activeEffects[i].name == "Paralyzed") {
            badgeBg = sf::Color(35, 30, 10, 230);
            badgeBorder = sf::Color(245, 215, 30, 220);
            textColor = sf::Color(255, 240, 140);
        }

        drawFramedBox(target, x, y, badgeW, badgeH, badgeBg, badgeBorder, 1.0f);

        char timerBuf[16];
        snprintf(timerBuf, sizeof(timerBuf), "%.1fs", activeEffects[i].timer);
        std::string label = activeEffects[i].name.substr(0, 4) + " " + timerBuf;
        drawText(target, "bold", x + 5.0f, y + 3.0f, label, 10, textColor);
    }
}

// =========================================================================
// 3. Echo Core Integrity Bar (Top-Center for Protect Chambers)
// =========================================================================

void HUD::drawEchoIntegrityBar(sf::RenderTarget& target) const {
    float barW = 340.0f;
    float barH = 22.0f;
    float barX = position.x + (fixedWidth - barW) / 2.0f;
    float barY = position.y + 20.0f;

    sf::Color frameBorder = sf::Color(0, 180, 160, 200);
    if (displayedEchoRatio < 0.30f) {
        uint8_t pulseAlpha = static_cast<uint8_t>(140.0f + 110.0f * std::sin(animTime * 10.0f));
        frameBorder = sf::Color(255, 45, 45, pulseAlpha);
    }

    drawFramedBox(target, barX, barY, barW, barH,
                  sf::Color(8, 14, 20, 220), frameBorder, 1.0f);

    sf::Color gaugeFill = (displayedEchoRatio < 0.30f) ? sf::Color(240, 60, 40) : sf::Color(0, 220, 180);
    drawGlossyBar(target, barX, barY, barW, barH,
                  displayedEchoRatio, 0.0f,
                  sf::Color(8, 25, 30, 200),
                  gaugeFill,
                  sf::Color::Transparent,
                  frameBorder);

    int echoPct = static_cast<int>(std::round(displayedEchoRatio * 100.0f));
    std::string text = "ECHO " + std::to_string(echoPct) + "%";
    drawText(target, "bold", barX + 10.0f, barY + 3.0f, text, 11, sf::Color(230, 255, 250));
}

} // namespace UI

