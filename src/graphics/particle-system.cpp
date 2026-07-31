#include "particle-system.hpp"
#include <cmath>
#include <cstdlib>
#include <cstdint>

// Helper for random float between min and max
static float randomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (max - min));
}

ParticleSystem& ParticleSystem::getInstance() {
    static ParticleSystem instance;
    return instance;
}

ParticleSystem::ParticleSystem() : m_vertices(sf::PrimitiveType::Triangles), needsVertexUpdate(false) {}

void ParticleSystem::update(float dt) {
    needsVertexUpdate = false;
    for (size_t i = 0; i < particles.size(); ) {
        Particle& p = particles[i];
        p.lifetime -= dt;
        if (p.lifetime <= 0.0f) {
            // Remove particle
            particles[i] = particles.back();
            particles.pop_back();
            needsVertexUpdate = true;
        } else {
            p.position += p.velocity * dt;
            
            // Basic drag
            p.velocity *= std::pow(0.5f, dt);
            
            ++i;
            needsVertexUpdate = true; // Position changed
        }
    }

    if (needsVertexUpdate) {
        updateVertices();
    }
}

void ParticleSystem::updateVertices() {
    m_vertices.resize(particles.size() * 6); // 2 triangles per quad

    for (size_t i = 0; i < particles.size(); ++i) {
        Particle& p = particles[i];
        
        // Calculate alpha based on lifetime
        float ratio = p.lifetime * p.maxLifetimeInverse;
        sf::Color currentColor = p.color;
        
        if (p.isSparkle) {
            // Sparkle effect (pulsing alpha and size)
            float pulse = (std::sin(ratio * 15.0f) + 1.0f) * 0.5f;
            currentColor.a = static_cast<std::uint8_t>(currentColor.a * ratio * pulse);
        } else {
            currentColor.a = static_cast<std::uint8_t>(currentColor.a * ratio);
        }

        float halfSize = p.size * 0.5f;
        if (p.isSparkle) {
            halfSize *= (std::sin(ratio * 20.0f) + 1.2f) * 0.5f;
        }

        sf::Vector2f pos = p.position;

        sf::Vector2f tl = pos + sf::Vector2f(-halfSize, -halfSize);
        sf::Vector2f tr = pos + sf::Vector2f(halfSize, -halfSize);
        sf::Vector2f br = pos + sf::Vector2f(halfSize, halfSize);
        sf::Vector2f bl = pos + sf::Vector2f(-halfSize, halfSize);

        // Triangle 1 (TL, TR, BR)
        m_vertices[i * 6 + 0].position = tl;
        m_vertices[i * 6 + 1].position = tr;
        m_vertices[i * 6 + 2].position = br;
        
        // Triangle 2 (TL, BR, BL)
        m_vertices[i * 6 + 3].position = tl;
        m_vertices[i * 6 + 4].position = br;
        m_vertices[i * 6 + 5].position = bl;

        for (int v = 0; v < 6; ++v) {
            m_vertices[i * 6 + v].color = currentColor;
        }
    }
}

void ParticleSystem::emitBurst(sf::Vector2f pos, int count, sf::Color color, float speedMin, float speedMax, float lifeMin, float lifeMax, float size) {
    for (int i = 0; i < count; ++i) {
        float angle = randomFloat(0.0f, 2.0f * 3.14159f);
        float speed = randomFloat(speedMin, speedMax);
        float life = randomFloat(lifeMin, lifeMax);
        
        Particle p;
        p.position = pos;
        p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
        p.lifetime = life;
        p.maxLifetime = life;
        p.maxLifetimeInverse = 1.0f / life;
        p.color = color;
        p.size = size;
        p.isSparkle = false;
        
        particles.push_back(p);
    }
    needsVertexUpdate = true;
}

void ParticleSystem::emitSparkle(sf::Vector2f pos, int count, sf::Color color, float radius) {
    for (int i = 0; i < count; ++i) {
        float angle = randomFloat(0.0f, 2.0f * 3.14159f);
        float r = randomFloat(0.0f, radius);
        float life = randomFloat(0.5f, 1.2f);
        
        Particle p;
        p.position = pos + sf::Vector2f(std::cos(angle) * r, std::sin(angle) * r);
        p.velocity = sf::Vector2f(0.0f, randomFloat(-20.0f, 10.0f)); // Drift slightly upwards
        p.lifetime = life;
        p.maxLifetime = life;
        p.maxLifetimeInverse = 1.0f / life;
        p.color = color;
        p.size = randomFloat(2.0f, 5.0f);
        p.isSparkle = true;
        
        particles.push_back(p);
    }
    needsVertexUpdate = true;
}

void ParticleSystem::emitGlow(sf::Vector2f pos, int count, sf::Color color, float radius) {
    for (int i = 0; i < count; ++i) {
        float angle = randomFloat(0.0f, 2.0f * 3.14159f);
        float r = randomFloat(0.0f, radius);
        float speed = randomFloat(10.0f, 40.0f);
        float life = randomFloat(0.8f, 1.5f);
        
        Particle p;
        p.position = pos + sf::Vector2f(std::cos(angle) * r, std::sin(angle) * r);
        p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
        p.lifetime = life;
        p.maxLifetime = life;
        p.maxLifetimeInverse = 1.0f / life;
        p.color = color;
        p.size = randomFloat(4.0f, 8.0f);
        p.isSparkle = false;
        
        particles.push_back(p);
    }
    needsVertexUpdate = true;
}

void ParticleSystem::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!particles.empty()) {
        target.draw(m_vertices, states);
    }
}
