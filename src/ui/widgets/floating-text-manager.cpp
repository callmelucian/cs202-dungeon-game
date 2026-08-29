#include "floating-text-manager.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace UI {

FloatingText::FloatingText(sf::Vector2f startPos,
                           const std::string& textStr,
                           sf::Color fillColor,
                           unsigned int fontSize,
                           const std::string& fontKey,
                           float duration,
                           sf::Vector2f velocity,
                           sf::Color outlineColor,
                           float outlineThickness)
    : position(startPos),
      velocity(velocity),
      baseFillColor(fillColor),
      baseOutlineColor(outlineColor),
      lifetime(duration),
      maxLifetime(duration)
{
    textComponent = std::make_unique<UI::Text>(fontKey, fontSize);
    textComponent->setString(textStr);
    textComponent->setFillColor(fillColor);
    textComponent->setOutlineColor(outlineColor);
    textComponent->setOutlineThickness(outlineThickness);
    textComponent->setPosition(startPos);
}

void FloatingText::update(float dt) {
    lifetime -= dt;
    position += velocity * dt;
    velocity.y *= 0.96f; // Smooth upward deceleration

    float alpha = std::clamp(lifetime / maxLifetime, 0.0f, 1.0f);
    sf::Color curFill = baseFillColor;
    curFill.a = static_cast<uint8_t>(baseFillColor.a * alpha);
    sf::Color curOutline = baseOutlineColor;
    curOutline.a = static_cast<uint8_t>(baseOutlineColor.a * alpha);

    if (textComponent) {
        textComponent->setFillColor(curFill);
        textComponent->setOutlineColor(curOutline);
        textComponent->setPosition(position);
    }
}

void FloatingText::draw(sf::RenderTarget& target) const {
    if (textComponent && lifetime > 0.0f) {
        textComponent->draw(target);
    }
}

bool FloatingText::isExpired() const {
    return lifetime <= 0.0f;
}

// ---- FloatingTextManager Implementation ----

FloatingTextManager& FloatingTextManager::getInstance() {
    static FloatingTextManager instance;
    return instance;
}

void FloatingTextManager::update(float dt) {
    for (auto it = activeTexts.begin(); it != activeTexts.end(); ) {
        it->update(dt);
        if (it->isExpired()) {
            it = activeTexts.erase(it);
        } else {
            ++it;
        }
    }
}

void FloatingTextManager::draw(sf::RenderTarget& target) const {
    for (const auto& text : activeTexts) {
        text.draw(target);
    }
}

void FloatingTextManager::clear() {
    activeTexts.clear();
}

void FloatingTextManager::spawnText(sf::Vector2f pos,
                                    const std::string& text,
                                    sf::Color color,
                                    unsigned int fontSize,
                                    const std::string& fontKey,
                                    float duration,
                                    sf::Vector2f velocity,
                                    sf::Color outlineColor,
                                    float outlineThickness)
{
    activeTexts.emplace_back(pos, text, color, fontSize, fontKey, duration, velocity, outlineColor, outlineThickness);
}

void FloatingTextManager::spawnDamage(sf::Vector2f pos, float amount, bool isCritical) {
    std::ostringstream ss;
    ss << "-" << static_cast<int>(std::round(amount));

    unsigned int size = isCritical ? 14 : 9;
    sf::Color fill = isCritical ? sf::Color(255, 40, 40) : sf::Color::White; // Bright Red for critical hits, White for normal
    sf::Color outline = sf::Color(0, 0, 0, 240); // Solid black outline
    float duration = isCritical ? 1.1f : 0.75f;
    sf::Vector2f vel(0.0f, isCritical ? -70.0f : -50.0f);

    spawnText(pos, ss.str(), fill, size, "header", duration, vel, outline, isCritical ? 2.0f : 1.5f);
}

void FloatingTextManager::spawnHeal(sf::Vector2f pos, float amount) {
    std::ostringstream ss;
    ss << "+" << static_cast<int>(std::round(amount));

    sf::Color fill(50, 255, 120); // Emerald heal green
    sf::Color outline(10, 40, 20, 230);
    sf::Vector2f vel(0.0f, -45.0f);

    spawnText(pos, ss.str(), fill, 9, "header", 0.8f, vel, outline, 1.5f);
}

void FloatingTextManager::spawnStatus(sf::Vector2f pos, const std::string& statusName, sf::Color color) {
    sf::Color outline(0, 0, 0, 220);
    sf::Vector2f vel(0.0f, -40.0f);
    spawnText(pos, statusName, color, 8, "header", 0.9f, vel, outline, 1.5f);
}

void FloatingTextManager::spawnEchoCollected(sf::Vector2f pos, const std::string& echoName, float power) {
    std::ostringstream ss;
    ss << "+ ECHO COLLECTED: " << echoName << " (" << static_cast<int>(std::round(power)) << "%)";

    sf::Color fill(80, 240, 140); // Radiant Emerald Gold
    sf::Color outline(0, 0, 0, 240);
    sf::Vector2f vel(0.0f, -30.0f);

    spawnText(pos, ss.str(), fill, 10, "header", 2.2f, vel, outline, 1.5f);
}

void FloatingTextManager::spawnEchoStolen(sf::Vector2f pos, const std::string& echoName) {
    std::string text = "- ECHO STOLEN: " + echoName + "!";

    sf::Color fill(255, 70, 70); // Fiery Crimson
    sf::Color outline(0, 0, 0, 240);
    sf::Vector2f vel(0.0f, -30.0f);

    spawnText(pos, text, fill, 10, "header", 2.2f, vel, outline, 1.5f);
}

} // namespace UI
