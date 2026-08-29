#include "critical-boost-effect.hpp"
#include "../../ui/graphics/particle-system.hpp"
#include "../character.hpp"

CriticalBoostEffect::CriticalBoostEffect(float duration)
    : StatusEffect(duration) {}

void CriticalBoostEffect::apply(Character& character) {
    ParticleSystem::getInstance().emitBurst(character.getPosition(), 25, sf::Color(255, 50, 50, 220), 40.0f, 110.0f, 0.4f, 0.8f, 4.5f);
}

void CriticalBoostEffect::remove(Character& character) {
    // No-op - crit rate is handled dynamically in Player::getCriticalHitRate()
}

bool CriticalBoostEffect::update(float dt, Character& character) {
    return StatusEffect::update(dt, character);
}
