#include "boss-chamber.hpp"
#include "tilemap-loader.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../global-settings/sound-manager.hpp"
#include "../graphics/particle-system.hpp"
#include "../utils/collision-solver.hpp"
#include "../utils/math-utility.hpp"
#include "../entities/player.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <cstdint>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

BossChamber::BossChamber(Player& player)
    : Chamber(player),
      currentPhase(1),
      shrinkRate(0.1f),
      minRadius(1.5f)
{
    boss = std::make_unique<BossMalachar>(player);
    initPlatforms();
}

void BossChamber::setGrids2D5(const std::vector<std::vector<std::string>>& tGrid, const std::vector<std::vector<int>>& lGrid) {
    Chamber::setGrids2D5(tGrid, lGrid);
    initPlatforms();
}

void BossChamber::initPlatforms() {
    platforms.clear();

    float cellSize = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();
    float cols = SettingManager::getInstance().getGridCols();
    float rows = SettingManager::getInstance().getGridRows();

    if (!typeGrid.empty() && !typeGrid[0].empty()) {
        cols = static_cast<float>(typeGrid[0].size());
        rows = static_cast<float>(typeGrid.size());
    }

    sf::Vector2f centerPos = {ox + (cols * cellSize) / 2.0f, oy + (rows * cellSize) / 2.0f};

    // Position Boss Malachar at the upper-center of the arena floor
    if (boss) {
        sf::Vector2f bossSpawnPos = {ox + (cols / 2.0f) * cellSize, oy + 5.5f * cellSize};
        boss->setPosition(bossSpawnPos);
    }

    float ringRadius = 6.0f * cellSize;

    for (int i = 0; i < 6; ++i) {
        float angle = static_cast<float>(i) * (2.0f * M_PI / 6.0f);
        sf::Vector2f pCenter = centerPos + sf::Vector2f(std::cos(angle) * ringRadius, std::sin(angle) * ringRadius);

        Platform p;
        p.id = i;
        p.center = pCenter;
        p.radius = 3.0f; // 3 units radius for Phase 3
        p.isWarning = false;
        p.sunderTimer = 0.0f;
        p.isSundered = false;
        p.recoveryTimer = 0.0f;

        platforms.push_back(p);
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

    // Update Boss AI
    if (boss && boss->isAlive()) {
        boss->update(dt);
        boss->updateState(dt, *this);

        // Resolve boss collision against obstacles so it can't clip through walls
        CollisionSolver::resolveX(*boss, getObstaclesFor(boss.get()), dt);
        CollisionSolver::resolveY(*boss, getObstaclesFor(boss.get()), dt);
    }

    // Update platform state ( shrinking in Phase 4, sunder timers )
    updatePlatforms(dt);

    // Hazard check: player taking void damage if off-platform during Phase 3 & 4
    if (currentPhase >= 3 && player.isAlive()) {
        sf::Vector2f playerPos = player.getPosition();
        float cellSize = SettingManager::getInstance().getCellSize();
        bool onPlatform = false;

        for (const auto& p : platforms) {
            if (p.isSundered) continue;
            float dist = Math::distance(playerPos, p.center);
            if (dist <= p.radius * cellSize) {
                onPlatform = true;
                break;
            }
        }

        if (!onPlatform) {
            player.takeDamage(10.0f * dt); // 10 damage/sec while in void
        }
    }
}

void BossChamber::updatePhaseTransitionSequence(float dt) {
    transitionTimer -= dt;

    if (transitionStage == PhaseTransitionStage::FREEZE_AND_CLEAR) {
        if (transitionTimer <= 0.0f) {
            transitionStage = PhaseTransitionStage::ZOOM_OUT;
            transitionTimer = 0.8f;
        }
    } else if (transitionStage == PhaseTransitionStage::ZOOM_OUT) {
        if (transitionTimer <= 0.0f) {
            // Apply map extension layout for the new phase
            applyMapLayoutForPhase(pendingNewPhase);
            transitionStage = PhaseTransitionStage::FADE_LERP_ISLANDS;
            transitionTimer = 1.2f;
            fadeAlpha = 0.0f;
        }
    } else if (transitionStage == PhaseTransitionStage::FADE_LERP_ISLANDS) {
        fadeAlpha = std::min(1.0f, fadeAlpha + dt / 1.2f);
        if (transitionTimer <= 0.0f) {
            fadeAlpha = 1.0f;
            transitionStage = PhaseTransitionStage::ZOOM_IN;
            transitionTimer = 0.8f;
        }
    } else if (transitionStage == PhaseTransitionStage::ZOOM_IN) {
        if (transitionTimer <= 0.0f) {
            transitionStage = PhaseTransitionStage::NONE;
            std::cout << "[BossChamber] Cinematic Phase Transition Complete. Game unfreezes and resumes!\n";
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
    std::cout << "[BossChamber] Cinematic Phase Transition Triggered -> Phase " << newPhase << "\n";
}

void BossChamber::applyMapLayoutForPhase(int phase) {
    std::string mapPath;

    ChamberConfig initialConfig = MapLoader::loadChamber("assets/maps/level-4/boss.json");
    if (initialConfig.phaseMaps.count(phase)) {
        mapPath = initialConfig.phaseMaps[phase];
    } else {
        mapPath = "assets/maps/level-4/boss-phase-" + std::to_string(phase) + ".json";
    }

    ChamberConfig phaseCfg = MapLoader::loadChamber(mapPath);
    if (!phaseCfg.typeGrid.empty() && !phaseCfg.levelGrid.empty()) {
        typeGrid = phaseCfg.typeGrid;
        levelGrid = phaseCfg.levelGrid;

        // Re-synthesize 2.5D render data, recreate renderable tileMap, and rebuild collision obstacles from JSON!
        Chamber::setGrids2D5(typeGrid, levelGrid);

        if (phase == 3) {
            // Re-initialize 6 floating platforms
            for (auto& p : platforms) {
                p.radius = 3.0f;
                p.isSundered = false;
                p.isWarning = false;
            }
        }
        std::cout << "[BossChamber] Loaded Phase " << phase << " layout directly from JSON file: " << mapPath << "\n";
    } else {
        std::cerr << "[BossChamber] Could not load phase map JSON: " << mapPath << "\n";
    }
}

void BossChamber::updatePlatforms(float dt) {
    // Phase 4: Platforms shrink continuously at 0.1 units/sec down to floor 1.5 units
    if (currentPhase == 4) {
        for (auto& p : platforms) {
            if (!p.isSundered && p.radius > minRadius) {
                p.radius = std::max(minRadius, p.radius - shrinkRate * dt);
            }
        }
    }

    // Process platform sunder warnings & recovery
    for (auto& p : platforms) {
        if (p.isWarning) {
            p.sunderTimer -= dt;
            if (p.sunderTimer <= 0.0f) {
                p.isWarning = false;
                p.isSundered = true;
                p.recoveryTimer = 12.0f; // Platform collapses, recovers after 12s
                std::cout << "[BossChamber] Platform " << p.id << " sundered into void!\n";
            }
        } else if (p.isSundered) {
            p.recoveryTimer -= dt;
            if (p.recoveryTimer <= 0.0f) {
                p.isSundered = false;
                p.radius = (currentPhase == 4) ? minRadius : 3.0f;
                std::cout << "[BossChamber] Platform " << p.id << " reformed.\n";
            }
        }
    }
}

void BossChamber::sunderPlatformAt(const sf::Vector2f& pos) {
    int closestIdx = -1;
    float closestDist = 999999.0f;

    for (size_t i = 0; i < platforms.size(); ++i) {
        if (platforms[i].isSundered) continue;

        float dist = Math::distance(pos, platforms[i].center);
        if (dist < closestDist) {
            closestDist = dist;
            closestIdx = static_cast<int>(i);
        }
    }

    if (closestIdx != -1) {
        sunderPlatform(closestIdx);
    }
}

void BossChamber::sunderPlatform(int index) {
    if (index >= 0 && index < static_cast<int>(platforms.size())) {
        auto& p = platforms[index];
        if (!p.isSundered && !p.isWarning) {
            p.isWarning = true;
            p.sunderTimer = 3.0f; // 3 seconds telegraph
            std::cout << "[BossChamber] Platform " << p.id << " targeted for Sunder (3s warning)!\n";
        }
    }
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

const std::vector<Platform>& BossChamber::getPlatforms() const {
    return platforms;
}

BossMalachar* BossChamber::getBoss() const {
    return boss.get();
}

void BossChamber::onBossDefeated() {
    std::cout << "[BossChamber] Boss defeated! Chamber completed.\n";
    completeChamber();
}

int BossChamber::processPlayerAttack(const Hitbox& hitbox) {
    // Process attacks on regular enemies first
    int hits = Chamber::processPlayerAttack(hitbox);

    // Also check collision against the boss
    if (boss && boss->isAlive()) {
        if (CollisionSolver::checkCollision(hitbox, boss->getBounds())) {
            hits++;
            float damage = player.getEffectiveStats().damage;
            if (player.getStateMachine().getActiveState()) {
                damage = player.getStateMachine().getActiveState()->modifyOutgoingDamage(damage);
            }
            boss->takeDamage(damage);

            bool lethal = !boss->isAlive();
            onEnemyHit(boss.get(), lethal);

            if (lethal) {
                onBossDefeated();
            }
        }
    }
    return hits;
}

void BossChamber::draw(sf::RenderWindow& window) {
    // Draw background platforms for Phase 3 and 4
    if (currentPhase >= 3) {
        drawPlatforms(window);
    }

    // Draw regular chamber components
    Chamber::draw(window);

    // Draw boss if active
    if (boss && boss->isAlive()) {
        boss->draw(window);
    }

    // Draw fade-in overlay effect during island expansion transition
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

void BossChamber::drawPlatforms(sf::RenderWindow& window) {
    float cellSize = SettingManager::getInstance().getCellSize();

    for (const auto& p : platforms) {
        if (p.isSundered) continue;

        float radiusPx = p.radius * cellSize;
        sf::CircleShape platformShape(radiusPx);
        platformShape.setOrigin({radiusPx, radiusPx});
        platformShape.setPosition(p.center);

        // Platform fill and outline
        if (p.isWarning) {
            // Telegraphing red/orange warning fill
            platformShape.setFillColor(sf::Color(180, 50, 50, 200));
            platformShape.setOutlineColor(sf::Color(255, 100, 0, 255));
            platformShape.setOutlineThickness(4.0f);
        } else {
            // Dark obsidian floating platform fill
            platformShape.setFillColor(sf::Color(40, 40, 60, 220));
            platformShape.setOutlineColor(sf::Color(120, 100, 180, 255));
            platformShape.setOutlineThickness(2.0f);
        }

        window.draw(platformShape);
    }
}
