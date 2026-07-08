#include "character-animator.hpp"
#include "../../global-settings/animation-manager.hpp"
#include "../../global-settings/asset-manager.hpp"

// ---- CharacterAnimator implementation ----
CharacterAnimator::CharacterAnimator(const std::string& characterKey) 
    : sprite(nullptr),
      characterKey(characterKey),
      currentAnimationKey("idle"),
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
        if (frame.frameRegion.size.x > 0 && frame.frameRegion.size.y > 0) {
            sprite->setTextureRect(frame.frameRegion);
            // set origin to center
            sprite->setOrigin(sf::Vector2f(frame.frameRegion.size.x / 2.0f, frame.frameRegion.size.y / 2.0f));
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

void CharacterAnimator::update(float dt) {
    currentAnimationElapsed += dt;
    
    if (hitFlashTimer > 0.0f) {
        hitFlashTimer -= dt;
    }

    const AnimationSet& animSet = AnimationManager::getInstance().getAnimationSet(characterKey);
    if (!animSet.textureKey.empty()) {
        const Animation& anim = animSet.getAnimation(currentAnimationKey);
        const AnimationFrame& frame = anim.getFrame(currentAnimationElapsed);
        
        if (frame.frameRegion.size.x > 0 && frame.frameRegion.size.y > 0) {
            sprite->setTextureRect(frame.frameRegion);
            // Keep origin centered
            sprite->setOrigin(sf::Vector2f(frame.frameRegion.size.x / 2.0f, frame.frameRegion.size.y / 2.0f));
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