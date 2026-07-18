#include "character-animator.hpp"
#include "../../global-settings/animation-manager.hpp"
#include "../../global-settings/asset-manager.hpp"

#include <iostream>
#include <cmath>

// ---- CharacterAnimator implementation ----
CharacterAnimator::CharacterAnimator(const std::string& characterKey) 
    : sprite(nullptr),
      characterKey(characterKey),
      currentAnimationKey("idle-facing-right"),
      hitFlashTimer(0.0f),
      currentAnimationElapsed(0.0f) 
{
    const AnimationSet& animSet = AnimationManager::getInstance().getAnimationSet(characterKey);
    if (!animSet.textureKey.empty()) {
        const sf::Texture& texture = AssetManager::getInstance().getTexture(animSet.textureKey);
        sprite = std::make_unique<sf::Sprite>(texture);
        
        // set initial frame if available
        const Animation& anim = animSet.getAnimation(currentAnimationKey);
        const AnimationFrame& frame = anim.getFrame(0.0f);
        if (frame.frameRegion.size.x != 0 && frame.frameRegion.size.y != 0) {
            sprite->setTextureRect(frame.frameRegion);
            // set origin to center, using absolute size because mirrored frames have negative size
            sprite->setOrigin(sf::Vector2f(std::abs(frame.frameRegion.size.x) / 2.0f, std::abs(frame.frameRegion.size.y) / 2.0f));
        }
    }
}

void CharacterAnimator::onStateChanged(const Character& character, std::string animationKey) {
    if (currentAnimationKey != animationKey) {
        currentAnimationKey = animationKey;
        currentAnimationElapsed = 0.0f;
    }
}

void CharacterAnimator::onDamaged(const Character& character, float amount) {
    hitFlashTimer = 0.2f;
}

void CharacterAnimator::onDefeated(const Character& character) {
    // Play defeat animations / effects
    onStateChanged(character, "defeat");
}

void CharacterAnimator::update(float dt, float speedMultiplier) {
    if (currentAnimationKey.find("walk") != std::string::npos || 
        currentAnimationKey.find("run") != std::string::npos) {
        currentAnimationElapsed += dt * speedMultiplier;
    } else {
        currentAnimationElapsed += dt;
    }
    
    if (hitFlashTimer > 0.0f) {
        hitFlashTimer -= dt;
    }

    const AnimationSet& animSet = AnimationManager::getInstance().getAnimationSet(characterKey);
    if (!animSet.textureKey.empty()) {
        const Animation& anim = animSet.getAnimation(currentAnimationKey);
        const AnimationFrame& frame = anim.getFrame(currentAnimationElapsed);
        
        if (frame.frameRegion.size.x != 0 && frame.frameRegion.size.y != 0) {
            sprite->setTextureRect(frame.frameRegion);
            // Keep origin centered, using absolute size
            sprite->setOrigin(sf::Vector2f(std::abs(frame.frameRegion.size.x) / 2.0f, std::abs(frame.frameRegion.size.y) / 2.0f));
        }
    }
}

void CharacterAnimator::draw(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2f size) const {
    if (sprite) {
        sprite->setPosition(position);
        
        if (size.x > 0.f && size.y > 0.f) {
            sf::FloatRect localBounds = sprite->getLocalBounds();
            if (localBounds.size.x > 0.f && localBounds.size.y > 0.f) {
                sprite->setScale(sf::Vector2f(size.x / localBounds.size.x, size.y / localBounds.size.y));
            }
        } else {
            sprite->setScale(sf::Vector2f(1.f, 1.f));
        }

        if (hitFlashTimer > 0.0f) {
            sprite->setColor(sf::Color::Red);
        } else {
            sprite->setColor(sf::Color::White);
        }
        window.draw(*sprite);
    }
}

void CharacterAnimator::setCharacterKey(const std::string& newKey) {
    characterKey = newKey;
    const AnimationSet& animSet = AnimationManager::getInstance().getAnimationSet(characterKey);
    if (!animSet.textureKey.empty()) {
        const sf::Texture& texture = AssetManager::getInstance().getTexture(animSet.textureKey);
        if (sprite) {
            sprite->setTexture(texture, true);
        } else {
            sprite = std::make_unique<sf::Sprite>(texture);
        }
    }
}

bool CharacterAnimator::isCurrentAnimationFinished() const {
    const AnimationSet& animSet = AnimationManager::getInstance().getAnimationSet(characterKey);
    if (!animSet.textureKey.empty()) {
        const Animation& anim = animSet.getAnimation(currentAnimationKey);
        return anim.isFinished(currentAnimationElapsed);
    }
    return true;
}