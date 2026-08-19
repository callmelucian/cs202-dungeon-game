#include "boss-health-bar.hpp"
#include "../../entities/enemy/boss-malachar.hpp"
#include "../../global-settings/asset-manager.hpp"
#include <algorithm>
#include <cmath>

namespace UI {

BossHealthBar::BossHealthBar()
    : currentHp(1250.0f),
      maxHp(1250.0f),
      displayedHpRatio(1.0f),
      currentPhase(1),
      bossName("MALACHAR, ECHO LORD"),
      lerpSpeed(8.0f)
{
}

void BossHealthBar::updateBossState(const BossMalachar& boss) {
    currentHp = boss.getHp();
    maxHp = boss.getEffectiveStats().maxHp;
    currentPhase = boss.getCurrentPhase();
}

void BossHealthBar::update(float dt) {
    float targetRatio = (maxHp > 0.0f) ? std::clamp(currentHp / maxHp, 0.0f, 1.0f) : 0.0f;
    displayedHpRatio += (targetRatio - displayedHpRatio) * lerpSpeed * dt;
}

void BossHealthBar::handleEvent(const sf::Event& event) {
    // Non-interactive HUD element
}

void BossHealthBar::draw(sf::RenderTarget& target) const {
    float screenW = static_cast<float>(target.getSize().x);
    float screenH = static_cast<float>(target.getSize().y);
    float barWidth = 420.0f;
    float barHeight = 24.0f;
    float posX = screenW - barWidth - 30.0f;
    float posY = screenH - barHeight - 40.0f; // Lower right corner

    // Background container frame
    sf::RectangleShape bgFrame({barWidth + 16.0f, barHeight + 36.0f});
    bgFrame.setPosition({posX - 8.0f, posY - 22.0f});
    bgFrame.setFillColor(sf::Color(15, 10, 25, 230));
    bgFrame.setOutlineColor(sf::Color(210, 160, 40, 220)); // Golden border
    bgFrame.setOutlineThickness(2.0f);
    target.draw(bgFrame);

    // Boss Title Text
    const sf::Font& font = AssetManager::getInstance().getFont("header");
    sf::Text titleText(font, bossName, 16);
    titleText.setFillColor(sf::Color(255, 220, 120));
    titleText.setPosition({posX, posY - 20.0f});
    target.draw(titleText);

    // Phase Indicator Text
    std::string phaseStr = "PHASE " + std::to_string(currentPhase) + " / 4";
    sf::Text phaseText(font, phaseStr, 14);
    phaseText.setFillColor(sf::Color(200, 150, 255));
    float phaseWidth = phaseText.getLocalBounds().size.x;
    phaseText.setPosition({posX + barWidth - phaseWidth, posY - 18.0f});
    target.draw(phaseText);

    // Health Bar Background Track
    sf::RectangleShape hpTrack({barWidth, barHeight});
    hpTrack.setPosition({posX, posY + 6.0f});
    hpTrack.setFillColor(sf::Color(40, 20, 50, 240));
    hpTrack.setOutlineColor(sf::Color(100, 50, 140, 255));
    hpTrack.setOutlineThickness(1.0f);
    target.draw(hpTrack);

    // Health Bar Fill (Glowing Void / Crimson Gradient)
    if (displayedHpRatio > 0.001f) {
        float fillW = barWidth * displayedHpRatio;
        sf::RectangleShape hpFill({fillW, barHeight});
        hpFill.setPosition({posX, posY + 6.0f});
        
        // Color shifts from Magenta/Purple in Phase 1-2 to Crimson in Phase 3-4
        if (currentPhase >= 3) {
            hpFill.setFillColor(sf::Color(220, 40, 60, 240));
        } else {
            hpFill.setFillColor(sf::Color(160, 40, 220, 240));
        }
        target.draw(hpFill);
    }

    // Health numerical overlay (e.g., "937 / 1250")
    const sf::Font& regFont = AssetManager::getInstance().getFont("regular");
    std::string hpNumStr = std::to_string(static_cast<int>(std::ceil(currentHp))) + " / " + std::to_string(static_cast<int>(maxHp));
    sf::Text hpNumText(regFont, hpNumStr, 13);
    hpNumText.setFillColor(sf::Color::White);
    sf::FloatRect bounds = hpNumText.getLocalBounds();
    hpNumText.setPosition({posX + (barWidth - bounds.size.x) / 2.0f, posY + 8.0f});
    target.draw(hpNumText);
}

} // namespace UI
