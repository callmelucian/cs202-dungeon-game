#ifndef AURA_RENDERER_HPP
#define AURA_RENDERER_HPP

#include <SFML/Graphics.hpp>

class AuraRenderer {
public:
    static AuraRenderer& getInstance();

    AuraRenderer(const AuraRenderer&) = delete;
    AuraRenderer& operator=(const AuraRenderer&) = delete;
    AuraRenderer(AuraRenderer&&) = delete;
    AuraRenderer& operator=(AuraRenderer&&) = delete;

    void update(float dt);

    /// Draws a continuous smooth radial gradient aura with animated outward air waves.
    void drawAura(sf::RenderTarget& target,
                  const sf::Vector2f& center,
                  float radius,
                  const sf::Color& coreColor,
                  const sf::Color& edgeColor,
                  float intensity = 1.0f,
                  float speed = 1.2f,
                  float timeOverride = -1.0f);

    /// Convenience overload using a base color to compute harmonious core and edge tones.
    void drawAura(sf::RenderTarget& target,
                  const sf::Vector2f& center,
                  float radius,
                  const sf::Color& baseColor,
                  float intensity = 1.0f,
                  float speed = 1.2f,
                  float timeOverride = -1.0f);

    /// Draws an expanding radial pulse wave (e.g. for shockwave or ability detonation).
    void drawPulse(sf::RenderTarget& target,
                   const sf::Vector2f& center,
                   float currentRadius,
                   float maxRadius,
                   const sf::Color& baseColor,
                   float progress);

    /// Triggers a full-screen flash (e.g. for critical hits)
    void triggerScreenFlash(const sf::Color& color = sf::Color(255, 255, 255, 210), float duration = 0.14f);
    void clearScreenFlash() { flashTimer = 0.0f; }
    bool hasScreenFlash() const;
    void drawScreenFlash(sf::RenderTarget& target, const sf::Vector2u& windowSize) const;

private:
    AuraRenderer();
    ~AuraRenderer() = default;

    float globalTime;
    sf::Color flashColor;
    float flashDuration;
    float flashTimer;
};

#endif // AURA_RENDERER_HPP
