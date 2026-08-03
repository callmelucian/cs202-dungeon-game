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

static std::string formatDisplayName(const std::string& key) {
    if (key.empty()) return "Character";
    std::string result;
    bool capitalize = true;
    for (char ch : key) {
        if (ch == '_' || ch == '-') {
            result += ' ';
            capitalize = true;
        } else if (capitalize) {
            result += static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
            capitalize = false;
        } else {
            result += ch;
        }
    }
    return result;
}

// ---- Character implementation ----
Character::Character(const std::string& key)
    : characterKey(key),
      displayName(formatDisplayName(key)),
      position(0.0f, 0.0f),
      velocity(0.0f, 0.0f),
      knockbackVelocity(0.0f, 0.0f),
      animator(std::make_unique<CharacterAnimator>(key))
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

    // Apply knockback damping
    const float KNOCKBACK_FRICTION = 10.0f;
    if (Math::length(knockbackVelocity) > 0.1f) {
        knockbackVelocity -= knockbackVelocity * KNOCKBACK_FRICTION * deltaTime;
    } else {
        knockbackVelocity = {0.f, 0.f};
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

    if (healthBar) {
        Stats effStats = getEffectiveStats();
        healthBar->setHealth(getHp(), effStats.maxHp);
        healthBar->update(deltaTime);
    }
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

    float textBaseY = getPosition().y - getBounds().size.y / 2.0f - 14.0f;

    if (isAlive() && healthBar) {
        sf::Vector2f boundsSize = getBounds().size;
        float headTopY = getPosition().y - boundsSize.y / 2.0f;
        float barX = getPosition().x - healthBar->getSize().x / 2.0f;
        float barY = headTopY - healthBar->getSize().y - 1.0f;
        const_cast<UI::HealthBar*>(healthBar.get())->setPosition(sf::Vector2f(barX, barY));
        healthBar->draw(window);

        textBaseY = barY - 2.0f;
    }

    // Draw Name and Status above health bar
    try {
        const sf::Font& font = AssetManager::getInstance().getFont("regular");
        
        std::string titleStr = getDisplayName();
        for (const auto& effect : statusEffects) {
            if (dynamic_cast<SlowedEffect*>(effect.get())) {
                titleStr += " [Slowed]";
            }
        }
        
        if (!titleStr.empty()) {
            sf::Text text(font, titleStr, 10);
            text.setFillColor(sf::Color::White);
            sf::FloatRect bounds = text.getLocalBounds();
            text.setPosition({getPosition().x - bounds.size.x / 2.0f, textBaseY - bounds.size.y});
            // window.draw(text);
        }
    } catch (...) {
        // Font not loaded, skip drawing text
    }
}

std::string Character::getDisplayName() const {
    return displayName;
}

void Character::setDisplayName(const std::string& name) {
    displayName = name;
}

UI::HealthBar* Character::getHealthBar() {
    return healthBar.get();
}

const UI::HealthBar* Character::getHealthBar() const {
    return healthBar.get();
}

void Character::setHealthBar(std::unique_ptr<UI::HealthBar> bar) {
    healthBar = std::move(bar);
}

void Character::takeDamage(float rawAmount) {
    if (!isAlive()) return;
    float finalDmg = calculateMitigatedDamage(rawAmount);
    baseStats.hp -= finalDmg;
    std::cout << "Character took " << finalDmg << " dmg. HP left: " << baseStats.hp << "\n";
    
    for (auto observer : observers) {
        observer->onDamaged(*this, finalDmg);
        if (!isAlive()) {
            observer->onDefeated(*this);
        }
    }
}

void Character::heal(float amount) {
    if (!isAlive()) return;
    baseStats.hp = std::min(baseStats.hp + amount, baseStats.maxHp);
    std::cout << "Character healed " << amount << " HP. HP now: " << baseStats.hp << "\n";
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

sf::Vector2f Character::getKnockbackVelocity() const {
    return knockbackVelocity;
}

void Character::setKnockbackVelocity(const sf::Vector2f& kvel) {
    knockbackVelocity = kvel;
}

sf::Vector2f Character::getEffectiveVelocity() const {
    return velocity + knockbackVelocity;
}

void Character::applyKnockback(const sf::Vector2f& direction, float magnitude) {
    if (Math::length(direction) > 0.0f) {
        knockbackVelocity = Math::normalize(direction) * magnitude;
    }
}

void Character::onWallCollision() {
    // Default implementation does nothing
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
    float size = SettingManager::getInstance().getCharacterHitboxSize();
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
