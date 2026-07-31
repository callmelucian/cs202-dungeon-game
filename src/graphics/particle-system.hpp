#ifndef PARTICLE_SYSTEM_HPP
#define PARTICLE_SYSTEM_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <functional>

struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float lifetime;
    float maxLifetime;
    sf::Color color;
    float size;
    float maxLifetimeInverse; // optimization for fading
    bool isSparkle; // if true, maybe do a sine wave on size
};

class ParticleSystem : public sf::Drawable {
private:
    std::vector<Particle> particles;
    sf::VertexArray m_vertices;
    bool needsVertexUpdate;

    void updateVertices();

public:
    ParticleSystem();
    ~ParticleSystem() = default;

    // Singleton pattern for global access across the game
    static ParticleSystem& getInstance();

    // Update all particles
    void update(float dt);

    // Emitters
    void emitBurst(sf::Vector2f pos, int count, sf::Color color, float speedMin, float speedMax, float lifeMin, float lifeMax, float size);
    void emitSparkle(sf::Vector2f pos, int count, sf::Color color, float radius);
    void emitGlow(sf::Vector2f pos, int count, sf::Color color, float radius);

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

#endif // PARTICLE_SYSTEM_HPP
