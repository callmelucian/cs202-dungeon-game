#include "character.hpp"
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include "effects/paralyzed-effect.hpp"
#include "../utils/math-utility.hpp"
#include "animation/character-animator.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../global-settings/asset-manager.hpp"
#include "../ui/graphics/particle-system.hpp"
#include "../ui/widgets/floating-text-manager.hpp"
#include <random>
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
            // Spawn floating status text above character with matching color
            sf::Vector2f headPos = getPosition() + sf::Vector2f(0.0f, -35.0f);
            UI::FloatingTextManager::getInstance().spawnStatus(headPos, effect->getDisplayName(), effect->getColor());
            statusEffects.push_back(std::move(effect));
        }
    }
    pendingStatusEffects.clear();

    tickStatusEffects(deltaTime);

    // Update baseline tint from active negative status effects or frozen state (buffs do not tint sprite)
    if (isFrozen()) {
        setTint(sf::Color(100, 220, 255)); // Frost Cyan
    } else if (hasStatusEffect("Burned")) {
        setTint(sf::Color(255, 120, 100)); // Slight Red for Burn
    } else if (hasStatusEffect("Slowed")) {
        setTint(sf::Color(180, 200, 255)); // Soft blue for Slow
    } else if (hasStatusEffect("Paralyzed")) {
        setTint(sf::Color(255, 230, 120)); // Pale gold for Paralyzed
    } else {
        setTint(sf::Color::White);
    }

    if (healthBar) {
        Stats effStats = getEffectiveStats();
        healthBar->setHealth(getHp(), effStats.maxHp);
        healthBar->update(deltaTime);

        // Position health bar above the character's head
        sf::Vector2f boundsSize = getBounds().size;
        float headTopY = getPosition().y - boundsSize.y / 2.0f;
        float barX = getPosition().x - healthBar->getSize().x / 2.0f;
        float barY = headTopY - healthBar->getSize().y - 1.0f;
        healthBar->setPosition(sf::Vector2f(barX, barY));
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
        float spriteSize = SettingManager::getInstance().getCellSize() * 0.8f;
        // Offset the sprite so the bounding box acts more like the character's feet/lower body
        sf::Vector2f spritePos = getPosition();
        spritePos.y -= spriteSize * 0.15f; 
        animator->draw(window, spritePos, sf::Vector2f(spriteSize, spriteSize));
    } else {
        // Fallback shape
        sf::RectangleShape rect(getBounds().size);
        rect.setPosition(getBounds().position);
        rect.setFillColor(sf::Color(200, 50, 50, 150));
        window.draw(rect);
    }

    if (isAlive() && healthBar) {
        healthBar->draw(window);
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

void Character::takeDamage(float rawAmount, bool isCritical) {
    if (!isAlive()) return;
    float finalDmg = calculateMitigatedDamage(rawAmount);
    baseStats.hp -= finalDmg;
    std::cout << "[" << getDisplayName() << "] took " << finalDmg << " dmg" << (isCritical ? " (CRIT)" : "") << ". HP left: " << baseStats.hp << "\n";
    
    // Spawn floating damage text above character
    sf::Vector2f headPos = getPosition() + sf::Vector2f(0.0f, -35.0f);
    static std::mt19937 dmgRng(std::random_device{}());
    std::uniform_real_distribution<float> jitter(-10.0f, 10.0f);
    headPos.x += jitter(dmgRng);
    UI::FloatingTextManager::getInstance().spawnDamage(headPos, finalDmg, isCritical);

    auto observersCopy = observers;
    for (auto observer : observersCopy) {
        if (std::find(observers.begin(), observers.end(), observer) != observers.end()) {
            observer->onDamaged(*this, finalDmg);
            if (!isAlive()) {
                observer->onDefeated(*this);
            }
        }
    }
}

void Character::heal(float amount) {
    if (!isAlive()) return;
    baseStats.hp = std::min(baseStats.hp + amount, baseStats.maxHp);
    std::cout << "[" << getDisplayName() << "] healed " << amount << " HP. HP now: " << baseStats.hp << "\n";

    sf::Vector2f headPos = getPosition() + sf::Vector2f(0.0f, -35.0f);
    UI::FloatingTextManager::getInstance().spawnHeal(headPos, amount);
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

void Character::clearNegativeStatusEffects() {
    for (auto& effect : statusEffects) {
        if (effect && effect->getName() != "SpeedUp") {
            effect->remove(*this);
        }
    }
    statusEffects.erase(
        std::remove_if(statusEffects.begin(), statusEffects.end(),
            [](const std::unique_ptr<StatusEffect>& eff) {
                return eff && eff->getName() != "SpeedUp";
            }),
        statusEffects.end()
    );
    pendingStatusEffects.erase(
        std::remove_if(pendingStatusEffects.begin(), pendingStatusEffects.end(),
            [](const std::unique_ptr<StatusEffect>& eff) {
                return eff && eff->getName() != "SpeedUp";
            }),
        pendingStatusEffects.end()
    );
    if (isFrozen()) {
        setTint(sf::Color(100, 220, 255));
    } else if (!statusEffects.empty()) {
        setTint(statusEffects.back()->getColor());
    } else {
        setTint(sf::Color::White);
    }
    ParticleSystem::getInstance().emitBurst(getPosition(), 30, sf::Color(50, 255, 120, 220), 50.0f, 120.0f, 0.4f, 0.9f, 5.0f);
    std::cout << "[" << getDisplayName() << "] Antidote applied! Purged all negative status effects.\n";
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
    auto observersCopy = observers;
    for (auto observer : observersCopy) {
        if (std::find(observers.begin(), observers.end(), observer) != observers.end()) {
            observer->onStateChanged(*this, visualKey);
        }
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

void Character::setHp(float hp) {
    baseStats.hp = std::clamp(hp, 0.0f, baseStats.maxHp);
}

void Character::setMaxHp(float maxHp) {
    baseStats.maxHp = maxHp;
    baseStats.hp = std::min(baseStats.hp, baseStats.maxHp);
}

float Character::getSpeed() const {
    return getEffectiveStats().speed;
}

void Character::setSpeed(float speed) {
    baseStats.speed = speed;
}

sf::FloatRect Character::getBounds() const {
    float cellSize = SettingManager::getInstance().getCellSize();
    // Tighter rectangle: less width, slightly shorter height (focused on lower body)
    float width = cellSize * 0.45f;
    float height = cellSize * 0.5f;
    return sf::FloatRect({position.x - width / 2.0f, position.y - height / 2.0f}, {width, height});
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
    return hasStatusEffect("Slowed");
}

bool Character::hasStatusEffect(const std::string& name) const {
    for (const auto& effect : statusEffects) {
        if (effect && effect->getName() == name) {
            return true;
        }
    }
    return false;
}

void Character::setCharacterKey(const std::string& key) {
    if (characterKey != key) {
        characterKey = key;
        if (animator) {
            animator->setCharacterKey(key);
        }
    }
}

void Character::setTint(const sf::Color& color) {
    if (animator) {
        animator->setTint(color);
    }
}

sf::Color Character::getTint() const {
    return animator ? animator->getTint() : sf::Color::White;
}

bool Character::isFrozen() const {
    return isFrozenState;
}

void Character::setFrozen(bool frozen) {
    isFrozenState = frozen;
}

bool Character::isAnimationFinished() const {
    return animator ? animator->isCurrentAnimationFinished() : true;
}
