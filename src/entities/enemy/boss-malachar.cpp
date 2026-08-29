#include "boss-malachar.hpp"
#include "../animation/character-animator.hpp"
#include "../../chambers/boss-chamber.hpp"
#include "enemy-state.hpp"
#include "enemy-steering-strategy.hpp"
#include "sprinter.hpp"
#include "../player.hpp"
#include "../../core/game.hpp"
#include "../../core/run-state.hpp"
#include "../../utils/math-utility.hpp"
#include "../../global-settings/setting-manager.hpp"
#include "../../ui/graphics/particle-system.hpp"
#include "../../ui/graphics/aura-renderer.hpp"
#include "../../ui/widgets/floating-text-manager.hpp"
#include "../effects/burned-effect.hpp"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <random>

BossMalachar::BossMalachar(Player& player)
    : Enemy("boss_malachar", player),
      currentPhase(1),
      phaseTimer(0.0f),
      cycleTimer(0.0f),
      isVoidBoltCharging(false),
      voidBoltTelegraphTimer(0.0f),
      voidBoltTelegraphMax(1.5f),
      voidBoltsRemaining(0),
      voidBoltIntervalTimer(0.0f),
      summoningBurstFired(false),
      blinkTimer(6.0f),
      sunderCooldown(15.0f),
      soulLanceCooldown(10.0f),
      isSoulLanceCharging(false),
      soulLanceTelegraphTimer(0.0f),
      soulLanceTelegraphMax(2.0f),
      reflectWardActive(false),
      reflectWardCooldown(0.0f),
      foretellActive(false),
      pendingResonanceBurstTimer(-1.0f),
      pendingResonanceBurst(false)
{
    baseStats.hp = 1250.0f;
    baseStats.maxHp = 1250.0f;
    baseStats.damage = 14.0f;
    baseStats.speed = 2.0f;
    fragmentDropCount = 10;

    setSteeringStrategy(std::make_unique<SeekStrategy>());
    changeState(std::make_unique<IdleState>());
    setHealthBar(nullptr); // Head health bar removed; large top-right UI bar used instead

    applyRunStateModifiers();
    resetCycle();
}

void BossMalachar::applyRunStateModifiers() {
    RunState& runState = Game::getInstance().getRunState();

    // 1. Hollow Bell stolen → activate reflect ward in Phase 1 only
    if (currentPhase == 1) {
        if (runState.echoOutcomes[EchoType::HOLLOW_BELL] == EchoOutcome::STOLEN) {
            reflectWardActive = true;
            reflectWardCooldown = 0.0f;
            std::cout << "[BossMalachar] Hollow Bell Stolen: Reflect Ward ACTIVE for Phase 1!\n";
        } else {
            reflectWardActive = false;
        }
    } else {
        // Reflect ward expires automatically at Phase 1 -> Phase 2 transition
        reflectWardActive = false;
    }

    // 2. Foretell modifier from Clarity Shard:
    // Phase 1: active if Clarity Shard was collected fully intact
    // Phase 2+: active if Clarity Shard was collected at any power
    if (currentPhase == 1) {
        foretellActive = (runState.echoOutcomes[EchoType::CLARITY_SHARD] == EchoOutcome::COLLECTED) 
                      || runState.foretellPhase1;
    } else {
        foretellActive = (runState.echoOutcomes[EchoType::CLARITY_SHARD] == EchoOutcome::COLLECTED) 
                      || runState.foretellActive;
    }

    if (foretellActive) {
        std::cout << "[BossMalachar] Foretell ACTIVE: Extended telegraphs enabled!\n";
    }
}

void BossMalachar::resetCycle() {
    isVoidBoltCharging = true;
    
    voidBoltTelegraphMax = foretellActive ? 2.0f : 1.5f;
    voidBoltTelegraphTimer = voidBoltTelegraphMax;
    voidBoltsRemaining = 3;
    voidBoltIntervalTimer = 0.0f;
    summoningBurstFired = false;
}

void BossMalachar::update(float deltaTime) {
    sf::Vector2f myPos = getPosition();
    sf::Vector2f playerPos = getPlayer().getPosition();
    sf::Vector2f dir = playerPos - myPos;

    // Dynamically switch weapon spritesheet based on phase & active attack:
    // Staff for Void Bolt / spell casting in Phase 1 & 2; Sword for Soul Lance / Sunder in Phase 3 & 4
    if (isSoulLanceCharging || currentPhase >= 3) {
        setCharacterKey("boss_malachar_sword");
    } else {
        setCharacterKey("boss_malachar_staff");
    }

    if (isVoidBoltCharging || isSoulLanceCharging) {
        if (std::abs(dir.x) >= std::abs(dir.y)) {
            facingString = (dir.x > 0.f) ? "right" : "left";
            isFacingRight = (dir.x > 0.f);
        } else {
            facingString = (dir.y > 0.f) ? "down" : "up";
        }

        if (isVoidBoltCharging) {
            notifyStateChanged("thrust_oversize-facing-" + facingString);
        } else if (isSoulLanceCharging) {
            notifyStateChanged("slash_oversize-facing-" + facingString);
        }
        Character::update(deltaTime);
    } else {
        Enemy::update(deltaTime);
    }

    // Always update in-flight projectiles (purple orbs continue flying even when boss is frozen)
    updateProjectiles(deltaTime);
}

void BossMalachar::updateState(float dt, Chamber& chamber) {
    // 1. Phase transition threshold checks
    if (baseStats.hp <= baseStats.maxHp * 0.75f && currentPhase < 2) {
        transitionPhase(2);
    } else if (baseStats.hp <= baseStats.maxHp * 0.50f && currentPhase < 3) {
        transitionPhase(3);
    } else if (baseStats.hp <= baseStats.maxHp * 0.25f && currentPhase < 4) {
        transitionPhase(4);
    }

    BossChamber* bossChamber = dynamic_cast<BossChamber*>(&chamber);
    if (bossChamber && bossChamber->getCurrentPhase() != currentPhase) {
        bossChamber->setCurrentPhase(currentPhase);
    }

    // 2. Tick Hollow Bell reflect ward cooldown
    if (reflectWardCooldown > 0.0f) {
        reflectWardCooldown -= dt;
    }

    // 3. Pending Resonance Core burst update
    if (pendingResonanceBurst) {
        pendingResonanceBurstTimer -= dt;
        if (pendingResonanceBurstTimer <= 0.0f) {
            pendingResonanceBurst = false;
            float secondBurst = baseStats.hp * 0.08f;
            baseStats.hp = std::max(0.0f, baseStats.hp - secondBurst);
            std::cout << "[BossMalachar] Second Resonance Core burst dealt " << secondBurst << " damage to Malachar!\n";
        }
    }

    // 4. Phase 2-4 Modifiers: Marrow Echo regen
    if (currentPhase >= 2) {
        applyMarrowRegen(dt);
    }

    // 5. Phase 2-3 Modifiers: Obsidian Key blink (Disabled in Phase 4)
    if (currentPhase >= 2 && currentPhase <= 3) {
        performBlink(chamber);
    }

    // 6. Fixed 12-second repeating cycle (Void Bolt & Summoning Burst)
    updateVoidBoltCycle(dt, chamber);

    // 7. Phase 3 & 4: Platform Sunder attack every 15s
    if (currentPhase >= 3) {
        sunderCooldown -= dt;
        if (sunderCooldown <= 0.0f) {
            platformSunder(chamber);
            sunderCooldown = 15.0f;
        }
    }

    // 8. Phase 4: Soul Lance attack every 10s
    if (currentPhase == 4) {
        soulLanceCooldown -= dt;
        if (soulLanceCooldown <= 0.0f && !isSoulLanceCharging) {
            soulLance(chamber);
        }
        updateSoulLance(dt, chamber);
    }

    Enemy::updateState(dt, chamber);
}

void BossMalachar::updateVoidBoltCycle(float dt, Chamber& chamber) {
    cycleTimer += dt;
    if (cycleTimer >= 12.0f) {
        cycleTimer -= 12.0f;
        resetCycle();
    }

    // Void Bolt Charge Telegraph
    if (isVoidBoltCharging) {
        voidBoltTelegraphTimer -= dt;
        if (voidBoltTelegraphTimer <= 0.0f) {
            isVoidBoltCharging = false;
        }
    }

    // Void Bolt Firing Sequence (3 bolts, 0.8s apart)
    if (!isVoidBoltCharging && voidBoltsRemaining > 0) {
        voidBoltIntervalTimer -= dt;
        if (voidBoltIntervalTimer <= 0.0f) {
            sf::Vector2f myPos = getPosition();
            sf::Vector2f targetPos = getPlayer().getPosition();
            sf::Vector2f dir = Math::normalize(targetPos - myPos);

            VoidBolt bolt;
            bolt.position = myPos;
            bolt.velocity = dir * 350.0f; // Speed 350 px/s
            bolt.damage = 14.0f;
            bolt.lifetime = 4.0f;
            bolt.active = true;

            voidBolts.push_back(bolt);

            voidBoltsRemaining--;
            voidBoltIntervalTimer = 0.8f; // Next bolt in 0.8s
        }
    }

    // Summoning Burst at t = 6.0s in cycle
    if (cycleTimer >= 6.0f && !summoningBurstFired) {
        summoningBurstFired = true;
        sf::Vector2f myPos = getPosition();

        auto w1 = std::make_unique<Sprinter>(getPlayer(), false);
        w1->setPosition(myPos + sf::Vector2f(-40.0f, -40.0f));
        w1->setGuaranteedDrop("FREEZE_POTION");

        auto w2 = std::make_unique<Sprinter>(getPlayer(), false);
        w2->setPosition(myPos + sf::Vector2f(40.0f, 40.0f));
        w2->setGuaranteedDrop("RANDOM_NON_FREEZE");

        chamber.spawnEnemy(std::move(w1));
        chamber.spawnEnemy(std::move(w2));

        std::cout << "[BossMalachar] Summoning Burst: Spawned 2 Sprinters (1 Freeze Potion + 1 Randomized Potion guaranteed drops)!\n";
    }
}

void BossMalachar::updateSoulLance(float dt, Chamber& chamber) {
    if (!isSoulLanceCharging) return;

    soulLanceTargetPos = getPlayer().getPosition();
    soulLanceTelegraphTimer -= dt;

    if (soulLanceTelegraphTimer <= 0.0f) {
        isSoulLanceCharging = false;

        // Soul Lance deals 30 damage to player upon impact
        getPlayer().takeDamage(30.0f);
        std::cout << "[BossMalachar] Soul Lance struck Serin for 30 damage!\n";
    }
}

void BossMalachar::updateProjectiles(float dt) {
    sf::Vector2f playerPos = getPlayer().getPosition();

    for (auto& bolt : voidBolts) {
        if (!bolt.active) continue;

        bolt.position += bolt.velocity * dt;
        bolt.lifetime -= dt;

        if (bolt.lifetime <= 0.0f) {
            bolt.active = false;
            continue;
        }

        // Check collision with player (hitbox radius 15px for smaller orb)
        float dist = Math::distance(bolt.position, playerPos);
        if (dist < 15.0f) {
            getPlayer().takeDamage(bolt.damage);
            bolt.active = false;
            std::cout << "[BossMalachar] Void Bolt hit Serin for " << bolt.damage << " damage!\n";

            // 25% chance to inflict Burned status effect
            static std::mt19937 burnRng(std::random_device{}());
            std::uniform_real_distribution<float> burnDist(0.0f, 1.0f);
            if (burnDist(burnRng) < 0.25f) {
                getPlayer().applyStatusEffect(std::make_unique<BurnedEffect>(5.0f, 5.0f));
                std::cout << "[BossMalachar] Void Bolt inflicted Burned on Serin!\n";
            }
        }
    }

    // Remove inactive bolts
    voidBolts.erase(
        std::remove_if(voidBolts.begin(), voidBolts.end(), [](const VoidBolt& b) { return !b.active; }),
        voidBolts.end()
    );
}

void BossMalachar::applyMarrowRegen(float dt) {
    RunState& runState = Game::getInstance().getRunState();
    if (runState.echoOutcomes[EchoType::MARROW] == EchoOutcome::STOLEN) {
        // 2% MaxHP / sec = 25 HP / sec
        float regen = 25.0f * dt;
        baseStats.hp = std::min(baseStats.maxHp, baseStats.hp + regen);
    }
}

void BossMalachar::performBlink(Chamber& chamber) {
    RunState& runState = Game::getInstance().getRunState();
    if (runState.echoOutcomes[EchoType::OBSIDIAN_KEY] != EchoOutcome::STOLEN) return;

    blinkTimer -= 0.016f; // rough frame step or passed dt
    if (blinkTimer <= 0.0f) {
        // Randomize next blink between 6 and 9 seconds
        blinkTimer = 6.0f + static_cast<float>(std::rand() % 300) / 100.0f;

        float cellSize = SettingManager::getInstance().getCellSize();
        float ox = SettingManager::getInstance().getGridOffsetX();
        float oy = SettingManager::getInstance().getGridOffsetY();

        // Single arena center
        sf::Vector2f arenaCenter = {ox + 10.0f * cellSize, oy + 10.0f * cellSize};
        float angle = static_cast<float>(std::rand() % 360) * (3.14159265f / 180.0f);
        float radius = static_cast<float>(std::rand() % std::max(1, static_cast<int>(6.0f * cellSize)));
        sf::Vector2f newPos = arenaCenter + sf::Vector2f(std::cos(angle) * radius, std::sin(angle) * radius);

        ParticleSystem::getInstance().emitBurst(getPosition(), 20, sf::Color(140, 40, 220, 220), 40.0f, 90.0f, 0.2f, 0.6f, 3.0f);
        setPosition(newPos);
        ParticleSystem::getInstance().emitBurst(newPos, 20, sf::Color(140, 40, 220, 220), 40.0f, 90.0f, 0.2f, 0.6f, 3.0f);
        std::cout << "[BossMalachar] Blinked to new arena location (" << newPos.x << ", " << newPos.y << ")!\n";
    }
}

void BossMalachar::resonanceCoreBurst() {
    RunState& runState = Game::getInstance().getRunState();
    if (runState.echoOutcomes[EchoType::RESONANCE_CORE] == EchoOutcome::COLLECTED) {
        float burstDamage = baseStats.hp * 0.08f;
        baseStats.hp = std::max(0.0f, baseStats.hp - burstDamage);
        std::cout << "[BossMalachar] Resonance Core transition burst dealt " << burstDamage << " damage to Malachar!\n";

        // Double burst (~1s apart) only if Resonance Core was collected fully intact (>= 90%)
        if (runState.echoPowers.count(EchoType::RESONANCE_CORE) && runState.echoPowers[EchoType::RESONANCE_CORE] >= 90.0f) {
            pendingResonanceBurst = true;
            pendingResonanceBurstTimer = 1.0f;
        } else {
            pendingResonanceBurst = false;
        }
    }
}

int BossMalachar::getCurrentPhase() const {
    return currentPhase;
}

void BossMalachar::transitionPhase(int phase) {
    currentPhase = phase;
    std::cout << "[BossMalachar] Transitioning to Phase " << currentPhase << "!\n";
    
    // Update RunState modifiers for new phase
    applyRunStateModifiers();
    
    // Trigger Resonance Core transition burst
    resonanceCoreBurst();
}

void BossMalachar::platformSunder(Chamber& chamber) {
    std::cout << "[BossMalachar] Casting Platform Sunder!\n";
    BossChamber* bossChamber = dynamic_cast<BossChamber*>(&chamber);
    if (bossChamber) {
        sf::Vector2f playerPos = getPlayer().getPosition();
        bossChamber->sunderPlatformAt(playerPos);
    }
}

void BossMalachar::soulLance(Chamber& chamber) {
    std::cout << "[BossMalachar] Casting Soul Lance!\n";
    isSoulLanceCharging = true;

    soulLanceTelegraphMax = foretellActive ? 2.5f : 2.0f;
    soulLanceTelegraphTimer = soulLanceTelegraphMax;
    soulLanceStartPos = getPosition();
    soulLanceTargetPos = getPlayer().getPosition();
    soulLanceCooldown = 10.0f;
}

void BossMalachar::takeDamage(float rawAmount, bool isCritical) {
    if (!isAlive()) return;

    // By default, Boss Malachar is shielded and immune to damage unless frozen!
    if (isShielded()) {
        sf::Vector2f headPos = getPosition() + sf::Vector2f(0.0f, -50.0f);
        UI::FloatingTextManager::getInstance().spawnStatus(headPos, "SHIELDED", sf::Color(140, 200, 255));
        ParticleSystem::getInstance().emitBurst(getPosition(), 15, sf::Color(140, 200, 255, 220), 30.0f, 80.0f, 0.2f, 0.5f, 3.0f);
        std::cout << "[BossMalachar] Attack deflected! Boss is SHIELDED (can only be attacked while frozen)!\n";
        return;
    }

    // Hollow Bell reflect ward (Phase 1 only)
    if (currentPhase == 1 && reflectWardActive && reflectWardCooldown <= 0.0f) {
        reflectWardCooldown = 8.0f;
        float reflectedDmg = rawAmount * 0.20f;
        getPlayer().takeDamage(reflectedDmg);
        std::cout << "[BossMalachar] Hollow Bell Reflect Ward triggered! Reflected " 
                  << reflectedDmg << " damage back to Serin!\n";
    }

    Enemy::takeDamage(rawAmount, isCritical);
}

void BossMalachar::onDeath(Chamber* chamber) {
    std::cout << "[BossMalachar] Malachar has been defeated!\n";
    BossChamber* bossChamber = dynamic_cast<BossChamber*>(chamber);
    if (bossChamber) {
        bossChamber->onBossDefeated();
    }
}

sf::FloatRect BossMalachar::getBounds() const {
    float cellSize = SettingManager::getInstance().getCellSize();
    // 2x2 grid area bounding box (2x width and 2x height = 4x area compared to 1x1 player)
    float width = cellSize * 0.9f;
    float height = cellSize * 1.0f;
    sf::Vector2f pos = getPosition();
    return sf::FloatRect({pos.x - width / 2.0f, pos.y - height / 2.0f}, {width, height});
}

void BossMalachar::draw(sf::RenderWindow& window) const {
    // 0. Draw protective shield barrier (active when not frozen)
    if (isAlive() && !isFrozen()) {
        float radius = 58.0f;
        // Smooth radial gradient cyan air shield barrier
        sf::Color coreColor(140, 220, 255, 110); // Bright crystalline cyan core
        sf::Color edgeColor(80, 160, 255, 170);   // Deep icy protective air boundary
        AuraRenderer::getInstance().drawAura(window, getPosition(), radius, coreColor, edgeColor, 0.95f, 1.2f);
    }

    // 1. Draw Void Bolt charge glow around Malachar (purple aura)
    if (isVoidBoltCharging) {
        float radius = 65.0f;
        // Smooth radial gradient void charging air aura (purple)
        sf::Color coreColor(220, 120, 255, 140); // Luminous magenta-violet charging core
        sf::Color edgeColor(130, 20, 200, 200);  // Dark void pulsing perimeter air
        AuraRenderer::getInstance().drawAura(window, getPosition(), radius, coreColor, edgeColor, 1.2f, 2.0f);
    }

    // 2. Draw Soul Lance charging: red aura around boss, aiming laser, and red target orb
    if (isSoulLanceCharging) {
        // Red radial charging air aura around Malachar
        float radius = 65.0f;
        sf::Color redCore(255, 80, 80, 150);
        sf::Color redEdge(210, 20, 20, 210);
        AuraRenderer::getInstance().drawAura(window, getPosition(), radius, redCore, redEdge, 1.25f, 2.2f);

        // Aim line
        sf::Vertex line[] = {
            sf::Vertex(getPosition(), sf::Color(255, 40, 40, 230)),
            sf::Vertex(soulLanceTargetPos, sf::Color(255, 80, 0, 255))
        };
        window.draw(line, 2, sf::PrimitiveType::Lines);

        // Glowing red target orb
        float targetRadius = 16.0f;
        sf::CircleShape targetMarker(targetRadius);
        targetMarker.setOrigin({targetRadius, targetRadius});
        targetMarker.setPosition(soulLanceTargetPos);
        targetMarker.setFillColor(sf::Color(255, 20, 20, 140));
        targetMarker.setOutlineColor(sf::Color(255, 100, 100, 240));
        targetMarker.setOutlineThickness(2.5f);
        window.draw(targetMarker);
    }

    // 3. Draw active Void Bolts (smaller purple orb)
    for (const auto& bolt : voidBolts) {
        if (!bolt.active) continue;
        float radius = 6.5f;
        sf::CircleShape bShape(radius);
        bShape.setOrigin({radius, radius});
        bShape.setPosition(bolt.position);
        bShape.setFillColor(sf::Color(150, 30, 230, 245));
        bShape.setOutlineColor(sf::Color(240, 180, 255, 255));
        bShape.setOutlineThickness(1.8f);
        window.draw(bShape);
    }

    // 4. Draw Malachar sprite scaled 2x larger (2x2 cell size = 1.6 * cellSize)
    if (animator && animator->hasSprite()) {
        float spriteSize = SettingManager::getInstance().getCellSize() * 1.6f;
        sf::Vector2f spritePos = getPosition();
        spritePos.y -= spriteSize * 0.15f;
        animator->draw(window, spritePos, sf::Vector2f(spriteSize, spriteSize));
    } else {
        sf::RectangleShape rect(getBounds().size);
        rect.setPosition(getBounds().position);
        rect.setFillColor(sf::Color(200, 50, 50, 150));
        window.draw(rect);
    }

    if (isAlive() && healthBar) {
        healthBar->draw(window);
    }
}

void BossMalachar::clearProjectiles() {
    voidBolts.clear();
    isVoidBoltCharging = false;
    isSoulLanceCharging = false;
}

void BossMalachar::cancelCharging() {
    isVoidBoltCharging = false;
    voidBoltTelegraphTimer = 0.0f;
    voidBoltsRemaining = 0;
    
    isSoulLanceCharging = false;
    soulLanceTelegraphTimer = 0.0f;
    
    setVelocity({0.0f, 0.0f});
    std::cout << "[BossMalachar] Attack charging canceled due to Freeze!\n";
}
