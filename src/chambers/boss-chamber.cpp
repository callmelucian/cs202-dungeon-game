#include "boss-chamber.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../utils/collision-solver.hpp"
#include "../utils/math-utility.hpp"
#include "../entities/player.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>

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

void BossChamber::initPlatforms() {
    platforms.clear();

    float cellSize = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();
    float cols = SettingManager::getInstance().getGridCols();
    float rows = SettingManager::getInstance().getGridRows();

    sf::Vector2f centerPos = {ox + (cols * cellSize) / 2.0f, oy + (rows * cellSize) / 2.0f};
    float ringRadius = 7.0f * cellSize;

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
    // Update underlying chamber ( regular enemies, items, collisions )
    Chamber::update(dt);

    // Update Boss AI
    if (boss && boss->isAlive()) {
        boss->update(dt);
        boss->updateState(dt, *this);
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
        std::cout << "[BossChamber] Phase changed to " << currentPhase << "\n";
        if (currentPhase == 3) {
            // Re-init / activate 6 floating platforms for Phase 3
            for (auto& p : platforms) {
                p.radius = 3.0f;
                p.isSundered = false;
                p.isWarning = false;
            }
        }
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
