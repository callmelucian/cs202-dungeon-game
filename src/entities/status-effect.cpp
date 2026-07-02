#include "status-effect.hpp"
#include "character.hpp"

StatusEffect::StatusEffect(float duration) : duration(duration), timer(0.0f) {}

bool StatusEffect::update(float dt, Character& character) {
    timer += dt;
    return timer < duration;
}

void StatusEffect::refresh(const StatusEffect& other) {
    timer = 0.0f;
    duration = other.duration;
}

float StatusEffect::getDuration() const {
    return duration;
}

float StatusEffect::getTimer() const {
    return timer;
}
