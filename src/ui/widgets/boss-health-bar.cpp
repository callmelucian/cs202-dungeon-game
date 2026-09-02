#include "boss-health-bar.hpp"
#include "../../entities/enemy/boss-malachar.hpp"
#include "../../global-settings/asset-manager.hpp"
#include "../../utils/math-utility.hpp"
#include <algorithm>
#include <cmath>

namespace UI {

static sf::Color getPhaseColor(int phase) {
    switch (phase) {
        case 1: return sf::Color(165, 55, 235); // Void Amethyst (Phase 1)
        case 2: return sf::Color(30, 185, 235);  // Astral Cyan (Phase 2)
        case 3: return sf::Color(245, 145, 25);  // Molten Amber (Phase 3)
        case 4: return sf::Color(230, 35, 50);   // Enraged Crimson (Phase 4)
        default: return sf::Color(20, 15, 25);
    }
}

BossHealthBar::BossHealthBar()
    : currentHp(1250.0f),
      maxHp(1250.0f),
      displayedHpRatio(1.0f),
      displayedPhaseRatio(1.0f),
      ghostPhaseRatio(1.0f),
      ghostLagTimer(0.0f),
      currentPhase(1),
      bossName("MALACHAR, ECHO LORD"),
      lerpSpeed(8.0f),
      animTime(0.0f)
{
}

void BossHealthBar::updateBossState(const BossMalachar& boss) {
    currentHp = boss.getHp();
    maxHp = boss.getEffectiveStats().maxHp;
    currentPhase = boss.getCurrentPhase();
}

void BossHealthBar::update(float dt) {
    if (dt <= 0.0f) return;
    animTime += dt;

    float t = 1.0f - std::exp(-lerpSpeed * dt);
    float targetTotalRatio = (maxHp > 0.0f) ? std::clamp(currentHp / maxHp, 0.0f, 1.0f) : 0.0f;
    displayedHpRatio = Math::lerp(displayedHpRatio, targetTotalRatio, t);

    // Calculate current phase health ratio (each phase is 25% of total Max HP)
    float hpPerPhase = (maxHp > 0.0f) ? (maxHp / 4.0f) : 1.0f;
    float phaseHp = 0.0f;
    if (currentPhase == 1) {
        phaseHp = currentHp - 3.0f * hpPerPhase;
    } else if (currentPhase == 2) {
        phaseHp = currentHp - 2.0f * hpPerPhase;
    } else if (currentPhase == 3) {
        phaseHp = currentHp - 1.0f * hpPerPhase;
    } else {
        phaseHp = currentHp;
    }
    float targetPhaseRatio = std::clamp(phaseHp / hpPerPhase, 0.0f, 1.0f);

    displayedPhaseRatio = Math::lerp(displayedPhaseRatio, targetPhaseRatio, t);

    // Damage ghost bar lag
    if (ghostPhaseRatio < displayedPhaseRatio) {
        ghostPhaseRatio = displayedPhaseRatio;
        ghostLagTimer = 0.0f;
    } else if (ghostPhaseRatio > displayedPhaseRatio) {
        ghostLagTimer += dt;
        if (ghostLagTimer >= 0.30f) {
            float ghostT = 1.0f - std::exp(-4.0f * dt);
            ghostPhaseRatio = Math::lerp(ghostPhaseRatio, displayedPhaseRatio, ghostT);
        }
    }
}

void BossHealthBar::handleEvent(const sf::Event& /*event*/) {
    // Non-interactive HUD element
}

void BossHealthBar::draw(sf::RenderTarget& target) const {
    float screenW = static_cast<float>(target.getSize().x);
    float barWidth = 480.0f;
    float barHeight = 22.0f;
    float posX = (screenW - barWidth) / 2.0f; // Centered horizontally
    float posY = 32.0f;                      // Middle-top positioning

    // Background container frame
    sf::RectangleShape bgFrame({barWidth + 24.0f, barHeight + 36.0f});
    bgFrame.setPosition({posX - 12.0f, posY - 22.0f});
    bgFrame.setFillColor(sf::Color(12, 8, 20, 235));
    bgFrame.setOutlineColor(sf::Color(215, 165, 45, 220)); // Golden border
    bgFrame.setOutlineThickness(2.0f);
    target.draw(bgFrame);

    // Boss Title Text (Left-aligned)
    const sf::Font& font = AssetManager::getInstance().getFont("bold");
    sf::Text titleText(font, bossName, 13);
    titleText.setFillColor(sf::Color(255, 225, 120));
    titleText.setPosition({posX, posY - 19.0f});
    target.draw(titleText);

    // Phase Indicator Text & Phase Badges (Right-aligned, no overlap)
    std::string phaseStr = "PHASE " + std::to_string(currentPhase) + " / 4";
    sf::Text phaseText(font, phaseStr, 12);
    sf::Color currentPhaseColor = getPhaseColor(currentPhase);
    phaseText.setFillColor(currentPhaseColor);
    float phaseWidth = phaseText.getLocalBounds().size.x;
    phaseText.setPosition({posX + barWidth - phaseWidth - 52.0f, posY - 18.0f});
    target.draw(phaseText);

    // 4 Phase Layer Pips on the far right
    float pipX = posX + barWidth - 44.0f;
    float pipY = posY - 15.0f;
    for (int p = 1; p <= 4; ++p) {
        sf::RectangleShape pip({8.0f, 8.0f});
        pip.setPosition({pipX + (p - 1) * 11.0f, pipY});
        if (p < currentPhase) {
            // Already depleted phases: dimmed dark
            pip.setFillColor(sf::Color(40, 30, 50, 180));
            pip.setOutlineColor(sf::Color(70, 50, 80));
            pip.setOutlineThickness(1.0f);
        } else if (p == currentPhase) {
            // Active phase: glowing bright
            pip.setFillColor(getPhaseColor(p));
            pip.setOutlineColor(sf::Color(255, 255, 255, 240));
            pip.setOutlineThickness(1.5f);
        } else {
            // Upcoming phases: visible color preview
            pip.setFillColor(getPhaseColor(p));
            pip.setOutlineColor(sf::Color(30, 20, 35));
            pip.setOutlineThickness(1.0f);
        }
        target.draw(pip);
    }

    // 1. Next Phase Underlayer (Revealed as current layer depletes)
    sf::RectangleShape nextLayerTrack({barWidth, barHeight});
    nextLayerTrack.setPosition({posX, posY + 4.0f});
    if (currentPhase < 4) {
        nextLayerTrack.setFillColor(getPhaseColor(currentPhase + 1));
    } else {
        nextLayerTrack.setFillColor(sf::Color(25, 12, 18, 240)); // Dark empty track in final phase
    }
    nextLayerTrack.setOutlineColor(sf::Color(80, 40, 100, 220));
    nextLayerTrack.setOutlineThickness(1.0f);
    target.draw(nextLayerTrack);

    // 2. Ghost Damage Lag Fill
    if (ghostPhaseRatio > displayedPhaseRatio && ghostPhaseRatio > 0.001f) {
        float ghostW = std::clamp(barWidth * ghostPhaseRatio, 1.0f, barWidth);
        sf::RectangleShape ghostFill({ghostW, barHeight});
        ghostFill.setPosition({posX, posY + 4.0f});
        ghostFill.setFillColor(sf::Color(255, 220, 150, 200));
        target.draw(ghostFill);
    }

    // 3. Current Phase Health Bar Fill
    if (displayedPhaseRatio > 0.001f) {
        float fillW = std::clamp(barWidth * displayedPhaseRatio, 1.0f, barWidth);
        sf::RectangleShape hpFill({fillW, barHeight});
        hpFill.setPosition({posX, posY + 4.0f});
        hpFill.setFillColor(currentPhaseColor);
        target.draw(hpFill);

        // Subtle gloss line along the upper third
        sf::RectangleShape gloss({fillW, barHeight * 0.35f});
        gloss.setPosition({posX, posY + 4.0f});
        gloss.setFillColor(sf::Color(255, 255, 255, 40));
        target.draw(gloss);
    }

    // 4. Overall Frame Border around track
    sf::RectangleShape barBorder({barWidth, barHeight});
    barBorder.setPosition({posX, posY + 4.0f});
    barBorder.setFillColor(sf::Color::Transparent);
    barBorder.setOutlineColor(sf::Color(215, 165, 45, 180));
    barBorder.setOutlineThickness(1.0f);
    target.draw(barBorder);

    // 5. Health numerical overlay (e.g., "1238 / 1250") with drop shadow
    const sf::Font& regFont = AssetManager::getInstance().getFont("bold");
    std::string hpNumStr = std::to_string(static_cast<int>(std::ceil(currentHp))) + " / " + std::to_string(static_cast<int>(maxHp));
    
    sf::Text hpShadow(regFont, hpNumStr, 12);
    hpShadow.setFillColor(sf::Color(0, 0, 0, 220));
    sf::FloatRect bounds = hpShadow.getLocalBounds();
    float numX = posX + (barWidth - bounds.size.x) / 2.0f;
    float numY = posY + 6.0f;
    hpShadow.setPosition({numX + 1.0f, numY + 1.0f});
    target.draw(hpShadow);

    sf::Text hpNumText(regFont, hpNumStr, 12);
    hpNumText.setFillColor(sf::Color(255, 255, 255));
    hpNumText.setPosition({numX, numY});
    target.draw(hpNumText);
}

} // namespace UI

