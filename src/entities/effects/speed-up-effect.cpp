#include "speed-up-effect.hpp"
#include "../../ui/graphics/particle-system.hpp"
#include "../character.hpp"

SpeedUpEffect::SpeedUpEffect(float duration)
    : StatusEffect(duration, { .speedMultiplier = 1.5f }) {}

void SpeedUpEffect::apply(Character& character) {
    ParticleSystem::getInstance().emitBurst(character.getPosition(), 20, sf::Color(255, 215, 0, 220), 40.0f, 100.0f, 0.4f, 0.8f, 4.0f);
}

void SpeedUpEffect::remove(Character& character) {
    // No-op - speed is handled dynamically in getEffectiveStats()
}

bool SpeedUpEffect::update(float dt, Character& character) {
    return StatusEffect::update(dt, character);
}
