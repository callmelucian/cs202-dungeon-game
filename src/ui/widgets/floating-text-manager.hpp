#ifndef FLOATING_TEXT_MANAGER_HPP
#define FLOATING_TEXT_MANAGER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <string>
#include "../base/text.hpp"

namespace UI {

class FloatingText {
public:
    FloatingText(sf::Vector2f startPos,
                 const std::string& textStr,
                 sf::Color fillColor = sf::Color(255, 200, 50),
                 unsigned int fontSize = 14,
                 const std::string& fontKey = "header",
                 float duration = 0.8f,
                 sf::Vector2f velocity = sf::Vector2f(0.0f, -50.0f),
                 sf::Color outlineColor = sf::Color(0, 0, 0, 220),
                 float outlineThickness = 2.0f);

    void update(float dt);
    void draw(sf::RenderTarget& target) const;
    bool isExpired() const;

private:
    std::unique_ptr<UI::Text> textComponent;
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Color baseFillColor;
    sf::Color baseOutlineColor;
    float lifetime;
    float maxLifetime;
};

class FloatingTextManager {
public:
    static FloatingTextManager& getInstance();

    void update(float dt);
    void draw(sf::RenderTarget& target) const;
    void clear();

    // Generic spawn method
    void spawnText(sf::Vector2f pos,
                   const std::string& text,
                   sf::Color color = sf::Color::White,
                   unsigned int fontSize = 10,
                   const std::string& fontKey = "header",
                   float duration = 0.8f,
                   sf::Vector2f velocity = sf::Vector2f(0.0f, -50.0f),
                   sf::Color outlineColor = sf::Color(0, 0, 0, 220),
                   float outlineThickness = 2.0f);

    // Specialized helpers
    void spawnDamage(sf::Vector2f pos, float amount, bool isCritical = false);
    void spawnHeal(sf::Vector2f pos, float amount);
    void spawnStatus(sf::Vector2f pos, const std::string& statusName, sf::Color color);
    void spawnEchoCollected(sf::Vector2f pos, const std::string& echoName, float power);
    void spawnEchoStolen(sf::Vector2f pos, const std::string& echoName);

private:
    FloatingTextManager() = default;
    ~FloatingTextManager() = default;

    std::vector<FloatingText> activeTexts;
};

} // namespace UI

#endif // FLOATING_TEXT_MANAGER_HPP
