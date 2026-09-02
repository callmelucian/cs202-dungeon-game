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
      wraithbladeMomentum(0.0f),
      voidcasterMomentum(0.0f),
      ironshellMomentum(0.0f),
      displayedWraithblade(0.0f),
      displayedVoidcaster(0.0f),
      displayedIronshell(0.0f),
      switchCooldownTimer(0.0f),
      maxSwitchCooldown(SWITCH_COOLDOWN_DURATION),
      dashCooldownTimer(0.0f),
      maxDashCooldown(1.0f),
      echoPower(100.0f),
      displayedEchoRatio(1.0f),
      hasEcho(false),
      chamberLevel(1),
      chamberNumber(1),
      chamberTitle("The Drowned Archive"),
      elapsedChamberTime(0.0f),
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

    wraithbladeMomentum = player.getMomentum(FormType::WRAITHBLADE);
    voidcasterMomentum  = player.getMomentum(FormType::VOIDCASTER);
    ironshellMomentum   = player.getMomentum(FormType::IRONSHELL);
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

void HUD::updateChamberInfo(int level, int chamber, const std::string& title, float elapsedTime) {
    chamberLevel = level;
    chamberNumber = chamber;
    chamberTitle = title;
    elapsedChamberTime = elapsedTime;
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

    // Smooth 3 momentum bars (0..1 ratio)
    displayedWraithblade = Math::lerp(displayedWraithblade, std::clamp(wraithbladeMomentum / MAX_MOMENTUM, 0.0f, 1.0f), t);
    displayedVoidcaster  = Math::lerp(displayedVoidcaster,  std::clamp(voidcasterMomentum  / MAX_MOMENTUM, 0.0f, 1.0f), t);
    displayedIronshell   = Math::lerp(displayedIronshell,   std::clamp(ironshellMomentum   / MAX_MOMENTUM, 0.0f, 1.0f), t);

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
    drawChamberTimer(target);
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
// 1. Compact Player Cluster (Health + 3 Mini-Momentum Meters + Dash)
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

    // --- 3 Mini Momentum Bars (Side-by-side) ---
    float momY = startY + barH + 4.0f;
    float momBarW = 72.0f;
    float momBarH = 14.0f;
    float momGap = 7.0f;

    struct FormRow {
        FormType type;
        std::string key;
        float rawMom;
        float ratio;
        sf::Color activeCol;
        sf::Color frozenCol;
    };

    std::vector<FormRow> forms = {
        {FormType::WRAITHBLADE, "1:W", wraithbladeMomentum, displayedWraithblade, sf::Color(225, 50, 60), sf::Color(90, 30, 35, 180)},
        {FormType::VOIDCASTER,  "2:V", voidcasterMomentum,  displayedVoidcaster,  sf::Color(175, 65, 235), sf::Color(70, 30, 95, 180)},
        {FormType::IRONSHELL,   "3:I", ironshellMomentum,   displayedIronshell,   sf::Color(235, 185, 30), sf::Color(95, 75, 15, 180)}
    };

    for (size_t i = 0; i < forms.size(); ++i) {
        float x = barX + i * (momBarW + momGap);
        bool isActive = (activeForm == forms[i].type);

        sf::Color bgCol = isActive ? sf::Color(20, 25, 35, 240) : sf::Color(14, 16, 22, 200);
        sf::Color borderCol = isActive ? (forms[i].rawMom >= 50.0f ? sf::Color(255, 240, 100, 240) : forms[i].activeCol)
                                       : sf::Color(35, 42, 55, 160);

        drawFramedBox(target, x, momY, momBarW, momBarH, bgCol, borderCol, 1.0f);

        // Fill
        if (forms[i].ratio > 0.001f) {
            float fillW = std::clamp(momBarW * forms[i].ratio, 1.0f, momBarW);
            sf::RectangleShape fillRect({fillW, momBarH});
            fillRect.setPosition({x, momY});
            fillRect.setFillColor(isActive ? forms[i].activeCol : forms[i].frozenCol);
            target.draw(fillRect);
        }

        // Notch at 50%
        float notchX = x + momBarW * 0.50f;
        sf::RectangleShape notch({1.0f, momBarH});
        notch.setPosition({notchX, momY});
        notch.setFillColor(isActive && forms[i].rawMom >= 50.0f ? sf::Color(255, 255, 255, 220) : sf::Color(0, 0, 0, 100));
        target.draw(notch);

        // Label: "1:W 45" or "2:V 60"
        int intMom = static_cast<int>(std::round(forms[i].rawMom));
        std::string tag = forms[i].key + " " + std::to_string(intMom);
        sf::Color textCol = isActive ? sf::Color(255, 255, 255) : sf::Color(140, 155, 175);
        drawText(target, "regular", x + 4.0f, momY + 1.0f, tag, 10, textCol);
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
// 3. Minimal Chamber Timer (Top Right area)
// =========================================================================

void HUD::drawChamberTimer(sf::RenderTarget& target) const {
    float timerW = 76.0f;
    float timerH = 26.0f;
    float timerX = position.x + fixedWidth - timerW - 20.0f;
    float timerY = position.y + 82.0f;

    drawFramedBox(target, timerX, timerY, timerW, timerH,
                  sf::Color(10, 15, 24, 200),
                  sf::Color(40, 60, 85, 180), 1.0f);

    int totalSec = static_cast<int>(elapsedChamberTime);
    int minutes = totalSec / 60;
    int seconds = totalSec % 60;
    char timeStr[16];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", minutes, seconds);

    drawText(target, "pixel-bold", timerX + 11.0f, timerY + 4.0f, timeStr, 15, sf::Color(90, 230, 255));
}

// =========================================================================
// 4. Echo Core Integrity Bar (Top-Center for Protect Chambers)
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

