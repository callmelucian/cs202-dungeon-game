#include "boss-chamber.hpp"
#include "tilemap-loader.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../global-settings/sound-manager.hpp"
#include "../ui/graphics/particle-system.hpp"
#include "../ui/graphics/aura-renderer.hpp"
#include "../utils/camera.hpp"
#include "../ui/widgets/floating-text-manager.hpp"
#include "../utils/collision-solver.hpp"
#include "../utils/math-utility.hpp"
#include "../entities/player.hpp"
#include <cmath>
#include <algorithm>
#include <cstdint>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

BossChamber::BossChamber(Player& player)
    : Chamber(player),
      currentPhase(1)
{
    boss = std::make_unique<BossMalachar>(player);
}

void BossChamber::setGrids2D5(const std::vector<std::vector<std::string>>& tGrid, const std::vector<std::vector<int>>& lGrid, const std::vector<std::vector<std::string>>& bGrid) {
    Chamber::setGrids2D5(tGrid, lGrid, bGrid);

    float cellSize = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();

    // Position Boss Malachar at the upper-center of the arena floor
    if (boss) {
        sf::Vector2f bossSpawnPos = {ox + 10.0f * cellSize, oy + 5.0f * cellSize};
        boss->setPosition(bossSpawnPos);
    }
}

void BossChamber::update(float dt) {
    if (transitionStage != PhaseTransitionStage::NONE) {
        updatePhaseTransitionSequence(dt);
        ParticleSystem::getInstance().update(dt);
        return; // Freeze entities & player updates during cinematic transition!
    }

    // Update underlying chamber ( regular enemies, items, collisions )
    Chamber::update(dt);

    // Update Boss AI (movement & AI state paused if enemies are frozen)
    if (boss && boss->isAlive()) {
        if (isEnemiesFrozen()) {
            boss->setVelocity({0.0f, 0.0f});
            boss->update(dt);
        } else {
            if (boss->isFrozen()) {
                boss->setFrozen(false);
                boss->setTint(boss->getStatusEffects().empty() ? sf::Color::White : boss->getStatusEffects().back()->getColor());
            }
            boss->update(dt);
            boss->updateState(dt, *this);
        }

        // Resolve boss collision against obstacles so it can't clip through walls
        CollisionSolver::resolveX(*boss, getObstaclesFor(boss.get()), dt);
        CollisionSolver::resolveY(*boss, getObstaclesFor(boss.get()), dt);
    }

    // Update void rupture hazards (Sunder attack in Phase 3 & 4)
    updateRuptureZones(dt);

    // Hazard check: player taking void damage if standing within an active rupture zone
    if (player.isAlive()) {
        sf::Vector2f playerPos = player.getPosition();
        float cellSize = SettingManager::getInstance().getCellSize();

        bool inRupture = false;
        for (const auto& z : ruptureZones) {
            if (z.isActive) {
                float dist = Math::distance(playerPos, z.center);
                if (dist <= z.radius * cellSize) {
                    inRupture = true;
                    break;
                }
            }
        }

        if (inRupture) {
            sunderDamageTimer += dt;
            if (sunderDamageTimer >= 1.0f) {
                sunderDamageTimer -= 1.0f;
                player.takeDamage(10.0f); // 10 void damage tick per second
            }
        } else {
            sunderDamageTimer = 0.0f;
        }
    }
}

void BossChamber::updatePhaseTransitionSequence(float dt) {
    transitionTimer -= dt;

    if (transitionStage == PhaseTransitionStage::FREEZE_AND_CLEAR) {
        if (transitionTimer <= 0.0f) {
            transitionStage = PhaseTransitionStage::ZOOM_OUT;
            transitionTimer = 0.6f;
        }
    } else if (transitionStage == PhaseTransitionStage::ZOOM_OUT) {
        if (transitionTimer <= 0.0f) {
            transitionStage = PhaseTransitionStage::FADE_LERP_ISLANDS;
            transitionTimer = 0.8f;
            fadeAlpha = 0.0f;
        }
    } else if (transitionStage == PhaseTransitionStage::FADE_LERP_ISLANDS) {
        fadeAlpha = std::min(1.0f, fadeAlpha + dt / 0.8f);
        if (transitionTimer <= 0.0f) {
            fadeAlpha = 1.0f;
            transitionStage = PhaseTransitionStage::ZOOM_IN;
            transitionTimer = 0.6f;
        }
    } else if (transitionStage == PhaseTransitionStage::ZOOM_IN) {
        if (transitionTimer <= 0.0f) {
            transitionStage = PhaseTransitionStage::NONE;
        }
    }
}

void BossChamber::triggerPhaseTransition(int newPhase) {
    if (transitionStage != PhaseTransitionStage::NONE) return;
    pendingNewPhase = newPhase;
    transitionStage = PhaseTransitionStage::FREEZE_AND_CLEAR;
    transitionTimer = 0.4f;

    // 1. Resolve/clear temporary visual effects (particles, in-flight projectiles, debug hitboxes)
    ParticleSystem::getInstance().clear();
    if (boss) {
        boss->clearProjectiles();
    }
    debugHitboxes.clear();

    SoundManager::getInstance().playSound("boss-phase");
}

void BossChamber::updateRuptureZones(float dt) {
    for (auto it = ruptureZones.begin(); it != ruptureZones.end();) {
        if (it->isWarning) {
            it->warningTimer -= dt;
            if (it->warningTimer <= 0.0f) {
                it->isWarning = false;
                it->isActive = true;
                it->activeTimer = 12.0f; // 12 seconds active duration
                ParticleSystem::getInstance().emitBurst(it->center, 30, sf::Color(160, 40, 220, 220), 40.0f, 100.0f, 0.5f, 1.0f, 4.0f);
            }
            ++it;
        } else if (it->isActive) {
            it->activeTimer -= dt;
            if (it->activeTimer <= 0.0f) {
                it = ruptureZones.erase(it);
            } else {
                ++it;
            }
        } else {
            it = ruptureZones.erase(it);
        }
    }
}

void BossChamber::sunderPlatformAt(const sf::Vector2f& pos) {
    VoidRuptureZone zone;
    zone.center = pos;
    zone.radius = 3.0f; // 3.0 units radius
    zone.isWarning = true;
    zone.warningTimer = 3.0f; // 3.0s telegraph warning
    zone.isActive = false;
    zone.activeTimer = 12.0f;

    ruptureZones.push_back(zone);
}

int BossChamber::getCurrentPhase() const {
    return currentPhase;
}

void BossChamber::setCurrentPhase(int phase) {
    if (currentPhase != phase) {
        currentPhase = phase;
        triggerPhaseTransition(phase);
    }
}

BossMalachar* BossChamber::getBoss() const {
    return boss.get();
}

std::vector<Enemy*> BossChamber::getEnemiesRaw() const {
    std::vector<Enemy*> raw = Chamber::getEnemiesRaw();
    if (boss && boss->isAlive()) {
        raw.push_back(boss.get());
    }
    return raw;
}

void BossChamber::onEnemyHit(Enemy* enemy, bool lethal) {
    Chamber::onEnemyHit(enemy, lethal);
    if (enemy == boss.get() && lethal) {
        onBossDefeated();
    }
}

void BossChamber::onBossDefeated() {
    completeChamber();
}

void BossChamber::freezeAllEnemies(float duration) {
    Chamber::freezeAllEnemies(duration);
    if (boss && boss->isAlive()) {
        boss->cancelCharging();
        boss->setFrozen(true);
        boss->setTint(sf::Color(100, 220, 255));
        sf::Vector2f headPos = boss->getPosition() + sf::Vector2f(0.0f, -50.0f);
        UI::FloatingTextManager::getInstance().spawnStatus(headPos, "FROZEN", sf::Color(100, 220, 255));
        ParticleSystem::getInstance().emitBurst(boss->getPosition(), 40, sf::Color(100, 220, 255, 220), 40.0f, 100.0f, 0.6f, 1.2f, 5.0f);
    }
}

int BossChamber::processPlayerAttack(const Hitbox& hitbox) {
    // Process attacks on regular enemies first
    int hits = Chamber::processPlayerAttack(hitbox);

    // Also check collision against the boss
    if (boss && boss->isAlive()) {
        if (CollisionSolver::checkCollision(hitbox, boss->getBounds())) {
            hits++;
            float baseDamage = player.getEffectiveStats().damage;
            if (player.getStateMachine().getActiveState()) {
                baseDamage = player.getStateMachine().getActiveState()->modifyOutgoingDamage(baseDamage);
            }

            float critRate = player.getCriticalHitRate();
            bool isCrit = false;
            // Only consider critical hit if boss is NOT shielded (i.e. is frozen)
            if (!boss->isShielded() && critRate > 0.0f) {
                float roll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                if (roll < critRate) {
                    isCrit = true;
                }
            }

            float finalDamage = isCrit ? (baseDamage * 2.0f) : baseDamage;
            boss->takeDamage(finalDamage, isCrit);

            if (isCrit) {
                AuraRenderer::getInstance().triggerScreenFlash(sf::Color(255, 255, 255, 210), 0.14f);
                Camera::triggerShake(7.5f, 0.18f);
                ParticleSystem::getInstance().emitBurst(boss->getPosition(), 25, sf::Color(255, 60, 60, 240), 60.0f, 180.0f, 0.2f, 0.6f, 4.5f);
            }

            bool lethal = !boss->isAlive();
            onEnemyHit(boss.get(), lethal);

            if (lethal) {
                onBossDefeated();
            }
        }
    }
    return hits;
}

std::vector<sf::FloatRect> BossChamber::getObstaclesFor(const Character* character) const {
    return Chamber::getObstaclesFor(character);
}

void BossChamber::drawBackground(sf::RenderWindow& window) {
    Chamber::drawBackground(window);

    // Draw Void Sunder ground rupture hazard zones and telegraphs
    drawRuptureZones(window);
}

void BossChamber::draw(sf::RenderWindow& window) {
    // Draw regular chamber components (tilemap, background, items, enemies)
    Chamber::draw(window);

    // Draw boss if active
    if (boss && boss->isAlive()) {
        boss->draw(window);
    }

    // Draw fade-in overlay effect during phase transition
    if (transitionStage == PhaseTransitionStage::FADE_LERP_ISLANDS) {
        float cellSize = SettingManager::getInstance().getCellSize();
        float ox = SettingManager::getInstance().getGridOffsetX();
        float oy = SettingManager::getInstance().getGridOffsetY();
        float cols = static_cast<float>(typeGrid[0].size());
        float rows = static_cast<float>(typeGrid.size());

        sf::RectangleShape riftOverlay({cols * cellSize, rows * cellSize});
        riftOverlay.setPosition({ox, oy});
        std::uint8_t alpha = static_cast<std::uint8_t>(std::clamp((1.0f - fadeAlpha) * 160.0f, 0.0f, 255.0f));
        riftOverlay.setFillColor(sf::Color(160, 40, 220, alpha));
        window.draw(riftOverlay);
    }
}

void BossChamber::drawRuptureZones(sf::RenderWindow& window) {
    float cellSize = SettingManager::getInstance().getCellSize();

    for (const auto& z : ruptureZones) {
        float radiusPx = z.radius * cellSize;
        sf::CircleShape zoneShape(radiusPx);
        zoneShape.setOrigin({radiusPx, radiusPx});
        zoneShape.setPosition(z.center);

        if (z.isWarning) {
            // Telegraphing red/orange pulsing warning circle
            zoneShape.setFillColor(sf::Color(220, 60, 60, 120));
            zoneShape.setOutlineColor(sf::Color(255, 120, 0, 240));
            zoneShape.setOutlineThickness(3.0f);
            window.draw(zoneShape);
        } else if (z.isActive) {
            // Active dark void rupture pool with purple glowing border
            zoneShape.setFillColor(sf::Color(15, 5, 25, 210));
            zoneShape.setOutlineColor(sf::Color(160, 40, 220, 200));
            zoneShape.setOutlineThickness(3.0f);
            window.draw(zoneShape);
        }
    }
}
