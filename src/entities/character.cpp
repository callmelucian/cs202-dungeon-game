#include "character.hpp"
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include "effects/paralyzed-effect.hpp"
#include "../utils/math-utility.hpp"
#include "animation/character-animator.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../global-settings/asset-manager.hpp"
#include "effects/slowed-effect.hpp"

// ---- Character implementation ----
Character::Character(const std::string& characterKey)
    : position(0.0f, 0.0f),
      velocity(0.0f, 0.0f),
      animator(std::make_unique<CharacterAnimator>(characterKey))
{
    baseStats.hp = 100.0f;
    baseStats.maxHp = 100.0f;
    baseStats.damage = 10.0f;
    baseStats.speed = 100.0f;
    baseStats.defense = 0.0f;
    
    addObserver(animator.get());
}

Character::~Character() = default;

void Character::update(float deltaTime) {
    if (animator) {
        animator->update(deltaTime, getSpeed() / 5.0f);
    }

    // Process pending status effects to avoid iterator invalidation
    for (auto& effect : pendingStatusEffects) {
        bool found = false;
        for (auto& activeEffect : statusEffects) {
            if (typeid(*activeEffect) == typeid(*effect)) {
                activeEffect->refresh(*effect);
                found = true;
                break;
            }
        }
        if (!found) {
            effect->apply(*this);
            statusEffects.push_back(std::move(effect));
        }
    }
    pendingStatusEffects.clear();

    tickStatusEffects(deltaTime);
}

void Character::tickStatusEffects(float dt) {
    // Update active status effects and remove finished ones
    for (auto it = statusEffects.begin(); it != statusEffects.end(); ) {
        bool active = (*it)->update(dt, *this);
        if (!active) {
            (*it)->remove(*this);
            it = statusEffects.erase(it);
        }
        else ++it;
    }
}

void Character::draw(sf::RenderWindow &window) const {
    if (animator && animator->hasSprite()) {
        animator->draw(window, getPosition(), getBounds().size);
    } else {
        // Fallback shape
        sf::RectangleShape rect(getBounds().size);
        rect.setPosition(getBounds().position);
        rect.setFillColor(sf::Color(200, 50, 50, 150));
        window.draw(rect);
    }

    // Draw Status and HP above character
    try {
        const sf::Font& font = AssetManager::getInstance().getFont("regular");
        
        std::string infoStr = "HP: " + std::to_string(static_cast<int>(getHp()));
        for (const auto& effect : statusEffects) {
            // Check type for SlowedEffect
            if (dynamic_cast<SlowedEffect*>(effect.get())) {
                infoStr += " [Slowed]";
            }
        }
        
        sf::Text text(font, infoStr, 10);
        text.setFillColor(sf::Color::White);
        // text.setOutlineColor(sf::Color::Black);
        // text.setOutlineThickness(1.0f);
        
        sf::FloatRect bounds = text.getLocalBounds();
        text.setPosition({getPosition().x - bounds.size.x / 2.0f, getPosition().y - getBounds().size.y / 2.0f - 10.0f});
        
        window.draw(text);
    } catch (...) {
        // Font not loaded, skip drawing text
    }
}

void Character::takeDamage(float rawAmount) {
    float mitigated = calculateMitigatedDamage(rawAmount);
    baseStats.hp -= mitigated;
    if (baseStats.hp < 0.0f) {
        baseStats.hp = 0.0f;
    }

    for (auto observer : observers) {
        observer->onDamaged(*this, mitigated);
        if (!isAlive()) {
            observer->onDefeated(*this);
        }
    }
}

float Character::calculateMitigatedDamage(float rawAmount) {
    float defense = getEffectiveStats().defense;
    float mitigated = rawAmount * 100.0f / (100.0f + defense);
    return std::max(1.0f, std::round(mitigated));
}

void Character::applyStatusEffect(std::unique_ptr<StatusEffect> effect) {
    if (!effect) return;
    pendingStatusEffects.push_back(std::move(effect));
}

void Character::addObserver(CharacterObserver* observer) {
    if (observer && std::find(observers.begin(), observers.end(), observer) == observers.end()) {
        observers.push_back(observer);
    }
}

void Character::removeObserver(CharacterObserver* observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}

void Character::notifyStateChanged(std::string visualKey) {
    // std::cerr << "NOTIFY " << visualKey << std::endl;
    for (auto observer : observers) {
        observer->onStateChanged(*this, visualKey);
    }
}

sf::Vector2f Character::getPosition() const {
    return position;
}

void Character::setPosition(const sf::Vector2f& pos) {
    position = pos;
}

sf::Vector2f Character::getVelocity() const {
    return velocity;
}

void Character::setVelocity(const sf::Vector2f& vel) {
    velocity = vel;
}

float Character::getHp() const {
    return baseStats.hp;
}

float Character::getSpeed() const {
    return getEffectiveStats().speed;
}

void Character::setSpeed(float speed) {
    baseStats.speed = speed;
}

sf::FloatRect Character::getBounds() const {
    // A standard bounding box centered on the character's position
    float size = SettingManager::getInstance().getCharacterSize();
    return sf::FloatRect({position.x - size / 2, position.y - size / 2}, {size, size});
}

bool Character::canAct() const {
    return !Math::decide(getEffectiveStats().missChance);
}

Stats Character::getEffectiveStats() const {
    Stats stats = baseStats;
    for (const auto& effect : statusEffects) {
        stats = effect->getStatModifier().applyTo(stats);
    }
    return stats;
}

bool Character::isAlive() const {
    return baseStats.hp > 0.0f;
}

bool Character::isSlowed() const {
    for (const auto& effect : statusEffects) {
        if (dynamic_cast<SlowedEffect*>(effect.get())) {
            return true;
        }
    }
    return false;
}
