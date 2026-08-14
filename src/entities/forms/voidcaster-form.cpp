#include "voidcaster-form.hpp"
#include "../player.hpp"
#include "../../chambers/chamber.hpp"
#include "../../utils/math-utility.hpp"
#include "../../global-settings/sound-manager.hpp"
#include <cmath>

VoidcasterForm::VoidcasterForm()
    : PlayerForm(FormType::VOIDCASTER, "Voidcaster",
                 Stats{100.0f, 100.0f, 22.0f, 5.0f, 5.0f},
                 12.0f, 1.0f) {}

void VoidcasterForm::attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) {
    SoundManager::getInstance().playSound("shoot");
    
    // Voidcaster: Ranged (12 units, piercing) - Line shape
    float rangePixels = getAttackRange() * 60.0f; // 12.0 * 60 = 720
    
    float len = std::sqrt(targetDir.x * targetDir.x + targetDir.y * targetDir.y);
    if (len > 0) targetDir /= len;
    else targetDir = sf::Vector2f(1.0f, 0.0f);

    LineHitbox line;
    line.start = player.getPosition();
    line.end = line.start + targetDir * rangePixels;

    // Check hit enemies and distances before damage calculation
    int hits = 0;
    bool hasFarRangeHit = false;
    float farRangeThreshold = 6.0f * 60.0f; // 6 units = 360 pixels

    for (auto* enemy : chamber.getEnemiesRaw()) {
        if (!enemy || !enemy->isAlive()) continue;
        if (CollisionSolver::checkCollision(line, enemy->getBounds())) {
            hits++;
            float dist = Math::distance(player.getPosition(), enemy->getPosition());
            if (dist >= farRangeThreshold) {
                hasFarRangeHit = true;
            }
        }
    }

    chamber.processPlayerAttack(line);

    if (hits > 0) {
        // Gain +8 momentum for far-range hit (+4 bonus per additional pierced enemy)
        float baseGain = hasFarRangeHit ? 8.0f : 4.0f;
        player.gainMomentum(baseGain + 4.0f * (hits - 1), FormType::VOIDCASTER);
    }
}

std::unique_ptr<SpecialAbilityState> VoidcasterForm::createSpecialState(int abilityIndex) {
    if (abilityIndex == 1) {
        return std::make_unique<VoidcasterLanceState>(this);
    } else if (abilityIndex == 2) {
        return std::make_unique<VoidcasterDetonationFieldState>(this);
    }
    return nullptr;
}

float VoidcasterForm::getMomentumGainOnHit(float hpLost) const {
    return hpLost * 0.4f;
}

std::string VoidcasterForm::getAttackAnimKey() const {
    return "shoot-facing-";
}



// ---- VoidcasterLanceState ----
VoidcasterLanceState::VoidcasterLanceState(PlayerCombatState* inner)
    : SpecialAbilityState(inner, 1.0f) {}

StatModifier VoidcasterLanceState::getStatModifier() const {
    return StatModifier{};
}

const std::string& VoidcasterLanceState::getVisualKey() {
    static const std::string key = "VoidcasterLance";
    return key;
}

float VoidcasterLanceState::modifyOutgoingDamage(float baseAmount) {
    return baseAmount * 2.5f;
}

void VoidcasterLanceState::onAttack(Player& player, sf::Vector2f targetDir, Chamber& chamber) {
    float len = std::sqrt(targetDir.x * targetDir.x + targetDir.y * targetDir.y);
    if (len > 0) targetDir /= len;
    else targetDir = sf::Vector2f(1.0f, 0.0f);

    LineHitbox laser;
    laser.start = player.getPosition();
    laser.end = laser.start + (targetDir * 2000.0f); // 2000 pixels long!
    
    // TODO (Future): Update CollisionSolver to ignore wall collisions for this specific attack
    chamber.processPlayerAttack(laser);

    elapsedTime = duration; 
}

// ---- VoidcasterDetonationFieldState ----
VoidcasterDetonationFieldState::VoidcasterDetonationFieldState(PlayerCombatState* inner)
    : SpecialAbilityState(inner, 10.0f), isExplosionActive(false) {}

StatModifier VoidcasterDetonationFieldState::getStatModifier() const {
    return StatModifier{};
}

const std::string& VoidcasterDetonationFieldState::getVisualKey() {
    static const std::string key = "VoidcasterDetonationField";
    return key;
}

#include "../../graphics/particle-system.hpp"

void VoidcasterDetonationFieldState::onEnemyHit(Player& player, Enemy* enemy, bool lethal, Chamber& chamber) {
    if (enemy && !isExplosionActive) {
        isExplosionActive = true;
        
        // Detonation Field: 2.5 unit (150 pixel) radius explosion dealing 0.75x base damage
        CircleHitbox explosion;
        explosion.center = enemy->getPosition();
        explosion.radius = 2.5f * 60.0f; // 150.0f pixels

        float expDamage = player.getEffectiveStats().damage * 0.75f;
        
        for (auto* targetEnemy : chamber.getEnemiesRaw()) {
            if (targetEnemy && targetEnemy->isAlive()) {
                if (CollisionSolver::checkCollision(explosion, targetEnemy->getBounds())) {
                    targetEnemy->takeDamage(expDamage);
                    bool targetLethal = !targetEnemy->isAlive();
                    chamber.onEnemyHit(targetEnemy, targetLethal);
                }
            }
        }
        
        ParticleSystem::getInstance().emitBurst(enemy->getPosition(), 30, sf::Color(180, 80, 255, 220), 50.0f, 180.0f, 0.2f, 0.5f, 5.0f);
        isExplosionActive = false;
    }
    SpecialAbilityState::onEnemyHit(player, enemy, lethal, chamber);
}

void VoidcasterDetonationFieldState::draw(const Player& player, sf::RenderWindow& window) const {
    SpecialAbilityState::draw(player, window);

    float radius = 2.5f * 60.0f; // 150px
    sf::CircleShape field(radius);
    field.setOrigin({radius, radius});
    field.setPosition(player.getPosition());

    // Glowing void purple aura ring
    field.setFillColor(sf::Color(140, 50, 220, 30));
    field.setOutlineColor(sf::Color(190, 100, 255, 200));
    field.setOutlineThickness(3.0f);

    window.draw(field);
}
