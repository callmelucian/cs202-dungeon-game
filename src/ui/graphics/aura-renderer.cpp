#include "aura-renderer.hpp"
#include <cmath>
#include <algorithm>
#include <numbers>

AuraRenderer& AuraRenderer::getInstance() {
    static AuraRenderer instance;
    return instance;
}

AuraRenderer::AuraRenderer()
    : globalTime(0.0f),
      flashColor(sf::Color::White),
      flashDuration(0.0f),
      flashTimer(0.0f) {}

void AuraRenderer::update(float dt) {
    globalTime += dt;
    // Wrap around to avoid float precision loss over long play sessions
    if (globalTime > 10000.0f) {
        globalTime -= 10000.0f;
    }

    if (flashTimer > 0.0f) {
        flashTimer -= dt;
        if (flashTimer < 0.0f) {
            flashTimer = 0.0f;
        }
    }
}

void AuraRenderer::drawAura(sf::RenderTarget& target,
                           const sf::Vector2f& center,
                           float radius,
                           const sf::Color& coreColor,
                           const sf::Color& edgeColor,
                           float intensity,
                           float speed,
                           float timeOverride) {
    if (radius <= 0.0f || intensity <= 0.0f) return;

    float t = (timeOverride >= 0.0f) ? timeOverride : globalTime;

    constexpr int RINGS = 14;
    constexpr int SECTORS = 48;
    constexpr float TWO_PI = 6.28318530718f;

    // 1. Build smooth multi-ring radial gradient with harmonic convection and outward air waves
    sf::VertexArray mesh(sf::PrimitiveType::Triangles, RINGS * SECTORS * 6);

    auto evalPoint = [&](int ringIdx, int secIdx) -> std::pair<sf::Vector2f, sf::Color> {
        float u = static_cast<float>(ringIdx) / static_cast<float>(RINGS); // 0.0 at center, 1.0 at outer edge
        float theta = (static_cast<float>(secIdx) / static_cast<float>(SECTORS)) * TWO_PI;

        // Swirling atmospheric convection distortion (organic visible air turbulence)
        float convection = 0.035f * std::sin(4.0f * theta + 2.4f * t * speed)
                         + 0.025f * std::cos(3.0f * theta - 1.7f * t * speed);
        
        // Outward radial breathing
        float breathing = 1.0f + 0.02f * std::sin(t * speed * 2.0f);

        float currentR = u * radius * breathing * (1.0f + convection * u);
        sf::Vector2f pos = center + sf::Vector2f(std::cos(theta) * currentR, std::sin(theta) * currentR);

        // Smooth radial bell/cosine opacity profile (soft at center, rich mid-field, dissolved at edge)
        float baseCurve = std::sin(u * 3.14159265f);
        if (u < 0.15f) {
            // Keep center clear so character sprite stays crisp
            baseCurve = 0.35f + 0.65f * (u / 0.15f);
        } else {
            // Smooth exponential fadeout towards outer edge
            float fadeOut = std::clamp((1.0f - u) / 0.85f, 0.0f, 1.0f);
            baseCurve = fadeOut * fadeOut;
        }

        // Outward traveling visible air wave pulses
        // Waves travel outward from center (u=0) to edge (u=1)
        float wavePhase1 = u * 2.5f - t * speed * 0.85f;
        float wave1 = (std::sin(wavePhase1 * TWO_PI) + 1.0f) * 0.5f;

        float wavePhase2 = u * 4.0f - t * speed * 1.35f + 0.5f;
        float wave2 = (std::sin(wavePhase2 * TWO_PI) + 1.0f) * 0.5f;

        float airWaveIntensity = 0.70f + 0.30f * (0.65f * wave1 + 0.35f * wave2);

        // Color interpolation between core and edge
        float r = coreColor.r + (edgeColor.r - coreColor.r) * u;
        float g = coreColor.g + (edgeColor.g - coreColor.g) * u;
        float b = coreColor.b + (edgeColor.b - coreColor.b) * u;
        
        // Calculate final alpha
        float maxAlpha = std::max(coreColor.a, edgeColor.a);
        float computedAlpha = maxAlpha * baseCurve * airWaveIntensity * intensity;
        computedAlpha = std::clamp(computedAlpha, 0.0f, 255.0f);

        sf::Color vertexColor(
            static_cast<std::uint8_t>(std::clamp(r, 0.0f, 255.0f)),
            static_cast<std::uint8_t>(std::clamp(g, 0.0f, 255.0f)),
            static_cast<std::uint8_t>(std::clamp(b, 0.0f, 255.0f)),
            static_cast<std::uint8_t>(computedAlpha)
        );

        return {pos, vertexColor};
    };

    std::size_t vIndex = 0;
    for (int ring = 0; ring < RINGS; ++ring) {
        for (int sec = 0; sec < SECTORS; ++sec) {
            int nextSec = (sec + 1) % SECTORS;

            auto [p00, c00] = evalPoint(ring, sec);
            auto [p10, c10] = evalPoint(ring + 1, sec);
            auto [p11, c11] = evalPoint(ring + 1, nextSec);
            auto [p01, c01] = evalPoint(ring, nextSec);

            // Triangle 1
            mesh[vIndex + 0].position = p00;
            mesh[vIndex + 0].color = c00;
            mesh[vIndex + 1].position = p10;
            mesh[vIndex + 1].color = c10;
            mesh[vIndex + 2].position = p11;
            mesh[vIndex + 2].color = c11;

            // Triangle 2
            mesh[vIndex + 3].position = p00;
            mesh[vIndex + 3].color = c00;
            mesh[vIndex + 4].position = p11;
            mesh[vIndex + 4].color = c11;
            mesh[vIndex + 5].position = p01;
            mesh[vIndex + 5].color = c01;

            vIndex += 6;
        }
    }

    target.draw(mesh);

    // 2. Render discrete atmospheric wavefront rings (visible air ripples propagating outward)
    constexpr int WAVE_RINGS = 3;
    for (int w = 0; w < WAVE_RINGS; ++w) {
        // Offset each wave evenly across time
        float waveProgress = std::fmod(t * speed * 0.45f + static_cast<float>(w) / static_cast<float>(WAVE_RINGS), 1.0f);
        if (waveProgress <= 0.05f) continue;

        float ringRadius = waveProgress * radius;
        // Alpha peaks at mid-expansion, fades out cleanly near boundary
        float ringAlphaNorm = std::sin(waveProgress * 3.14159265f);
        float ringAlpha = edgeColor.a * ringAlphaNorm * 0.75f * intensity;

        if (ringAlpha > 1.0f) {
            sf::VertexArray ringLine(sf::PrimitiveType::TriangleStrip, (SECTORS + 1) * 2);
            float thickness = 2.0f + 2.5f * (1.0f - waveProgress);

            for (int s = 0; s <= SECTORS; ++s) {
                float theta = (static_cast<float>(s % SECTORS) / static_cast<float>(SECTORS)) * TWO_PI;
                float convection = 0.025f * std::sin(4.0f * theta + 2.4f * t * speed);
                float rInner = ringRadius * (1.0f + convection) - thickness * 0.5f;
                float rOuter = ringRadius * (1.0f + convection) + thickness * 0.5f;

                sf::Color ringCol = edgeColor;
                ringCol.a = static_cast<std::uint8_t>(std::clamp(ringAlpha, 0.0f, 255.0f));

                ringLine[s * 2 + 0].position = center + sf::Vector2f(std::cos(theta) * rInner, std::sin(theta) * rInner);
                ringLine[s * 2 + 0].color = ringCol;

                ringLine[s * 2 + 1].position = center + sf::Vector2f(std::cos(theta) * rOuter, std::sin(theta) * rOuter);
                ringLine[s * 2 + 1].color = ringCol;
            }
            target.draw(ringLine);
        }
    }
}

void AuraRenderer::drawAura(sf::RenderTarget& target,
                           const sf::Vector2f& center,
                           float radius,
                           const sf::Color& baseColor,
                           float intensity,
                           float speed,
                           float timeOverride) {
    // Derive brighter core color and softer outer edge color
    sf::Color coreColor(
        static_cast<std::uint8_t>(std::min(255, baseColor.r + 40)),
        static_cast<std::uint8_t>(std::min(255, baseColor.g + 40)),
        static_cast<std::uint8_t>(std::min(255, baseColor.b + 40)),
        static_cast<std::uint8_t>(baseColor.a * 0.7f)
    );

    sf::Color edgeColor = baseColor;
    edgeColor.a = static_cast<std::uint8_t>(baseColor.a * 0.9f);

    drawAura(target, center, radius, coreColor, edgeColor, intensity, speed, timeOverride);
}

void AuraRenderer::drawPulse(sf::RenderTarget& target,
                            const sf::Vector2f& center,
                            float currentRadius,
                            float maxRadius,
                            const sf::Color& baseColor,
                            float progress) {
    if (currentRadius <= 0.0f || progress >= 1.0f) return;

    float remaining = std::clamp(1.0f - progress, 0.0f, 1.0f);
    float alpha = baseColor.a * remaining;
    if (alpha <= 1.0f) return;

    constexpr int SECTORS = 64;
    constexpr float TWO_PI = 6.28318530718f;

    // 1. Draw smooth fading inner body
    sf::VertexArray body(sf::PrimitiveType::TriangleFan, SECTORS + 2);
    body[0].position = center;
    body[0].color = sf::Color(baseColor.r, baseColor.g, baseColor.b, static_cast<std::uint8_t>(alpha * 0.25f));

    for (int s = 0; s <= SECTORS; ++s) {
        float theta = (static_cast<float>(s % SECTORS) / static_cast<float>(SECTORS)) * TWO_PI;
        body[s + 1].position = center + sf::Vector2f(std::cos(theta) * currentRadius, std::sin(theta) * currentRadius);
        body[s + 1].color = sf::Color(baseColor.r, baseColor.g, baseColor.b, static_cast<std::uint8_t>(alpha * 0.7f));
    }
    target.draw(body);

    // 2. Draw shockwave rim ring
    sf::VertexArray rim(sf::PrimitiveType::TriangleStrip, (SECTORS + 1) * 2);
    float thickness = 4.0f + 6.0f * remaining;
    float rIn = std::max(0.0f, currentRadius - thickness * 0.5f);
    float rOut = currentRadius + thickness * 0.5f;

    sf::Color rimColor(
        static_cast<std::uint8_t>(std::min(255, baseColor.r + 50)),
        static_cast<std::uint8_t>(std::min(255, baseColor.g + 50)),
        static_cast<std::uint8_t>(std::min(255, baseColor.b + 50)),
        static_cast<std::uint8_t>(alpha)
    );

    for (int s = 0; s <= SECTORS; ++s) {
        float theta = (static_cast<float>(s % SECTORS) / static_cast<float>(SECTORS)) * TWO_PI;
        rim[s * 2 + 0].position = center + sf::Vector2f(std::cos(theta) * rIn, std::sin(theta) * rIn);
        rim[s * 2 + 0].color = rimColor;

        rim[s * 2 + 1].position = center + sf::Vector2f(std::cos(theta) * rOut, std::sin(theta) * rOut);
        rim[s * 2 + 1].color = sf::Color(rimColor.r, rimColor.g, rimColor.b, 0); // Fade out outer boundary
    }
    target.draw(rim);
}

void AuraRenderer::triggerScreenFlash(const sf::Color& color, float duration) {
    flashColor = color;
    flashDuration = duration;
    flashTimer = duration;
}

bool AuraRenderer::hasScreenFlash() const {
    return flashTimer > 0.0f;
}

void AuraRenderer::drawScreenFlash(sf::RenderTarget& target, const sf::Vector2u& windowSize) const {
    if (flashTimer <= 0.0f || flashDuration <= 0.0f) return;

    float progress = std::clamp(flashTimer / flashDuration, 0.0f, 1.0f);
    sf::RectangleShape flashRect(sf::Vector2f(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)));
    sf::Color drawColor = flashColor;
    drawColor.a = static_cast<std::uint8_t>(flashColor.a * progress);
    flashRect.setFillColor(drawColor);
    target.draw(flashRect);
}
