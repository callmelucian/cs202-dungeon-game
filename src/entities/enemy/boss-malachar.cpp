#include "boss-malachar.hpp"
#include "../../chambers/boss-chamber.hpp"
#include "enemy-state.hpp"
#include "enemy-steering-strategy.hpp"
#include "shard-wraith.hpp"
#include "../player.hpp"
#include "../../core/game.hpp"
#include "../../core/run-state.hpp"
#include "../../utils/math-utility.hpp"
#include "../../global-settings/setting-manager.hpp"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>

BossMalachar::BossMalachar(Player& player)
    : Enemy("boss_malachar", player),
      currentPhase(1),
      phaseTimer(0.0f),
      cycleTimer(0.0f),
      isVoidBoltCharging(false),
      voidBoltTelegraphTimer(0.0f),
      voidBoltTelegraphMax(0.5f),
      voidBoltsRemaining(0),
      voidBoltIntervalTimer(0.0f),
      summoningBurstFired(false),
      blinkTimer(6.0f),
      sunderCooldown(15.0f),
      soulLanceCooldown(10.0f),
      isSoulLanceCharging(false),
      soulLanceTelegraphTimer(0.0f),
      soulLanceTelegraphMax(1.0f),
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
    
    voidBoltTelegraphMax = foretellActive ? 1.1f : 0.5f;
    voidBoltTelegraphTimer = voidBoltTelegraphMax;
    voidBoltsRemaining = 3;
    voidBoltIntervalTimer = 0.0f;
    summoningBurstFired = false;
}

void BossMalachar::update(float deltaTime) {
    Enemy::update(deltaTime);
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

    // 9. Update active projectiles
    updateProjectiles(dt);

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

        auto w1 = std::make_unique<ShardWraith>(getPlayer());
        w1->setPosition(myPos + sf::Vector2f(-40.0f, -40.0f));

        auto w2 = std::make_unique<ShardWraith>(getPlayer());
        w2->setPosition(myPos + sf::Vector2f(40.0f, 40.0f));

        chamber.spawnEnemy(std::move(w1));
        chamber.spawnEnemy(std::move(w2));

        std::cout << "[BossMalachar] Summoning Burst: Spawned 2 Shard Wraiths!\n";
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

        // Check collision with player (hitbox radius 20px)
        float dist = Math::distance(bolt.position, playerPos);
        if (dist < 20.0f) {
            getPlayer().takeDamage(bolt.damage);
            bolt.active = false;
            std::cout << "[BossMalachar] Void Bolt hit Serin for " << bolt.damage << " damage!\n";
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

        BossChamber* bossChamber = dynamic_cast<BossChamber*>(&chamber);
        if (bossChamber && currentPhase == 3) {
            const auto& platforms = bossChamber->getPlatforms();
            std::vector<int> validIndices;
            for (size_t i = 0; i < platforms.size(); ++i) {
                if (!platforms[i].isSundered) validIndices.push_back(static_cast<int>(i));
            }
            if (!validIndices.empty()) {
                int chosen = validIndices[std::rand() % validIndices.size()];
                setPosition(platforms[chosen].center);
                std::cout << "[BossMalachar] Blinked to Platform " << chosen << "\n";
                return;
            }
        }

        // Default blink position around center
        float angle = static_cast<float>(std::rand() % 360) * (3.14159265f / 180.0f);
        float radius = 100.0f + static_cast<float>(std::rand() % 200);
        sf::Vector2f currentPos = getPosition();
        sf::Vector2f newPos = currentPos + sf::Vector2f(std::cos(angle) * radius, std::sin(angle) * radius);
        setPosition(newPos);
        std::cout << "[BossMalachar] Blinked to new arena location!\n";
    }
}

void BossMalachar::resonanceCoreBurst() {
    RunState& runState = Game::getInstance().getRunState();
    if (runState.echoOutcomes[EchoType::RESONANCE_CORE] == EchoOutcome::COLLECTED) {
        float burstDamage = baseStats.hp * 0.08f;
        baseStats.hp = std::max(0.0f, baseStats.hp - burstDamage);
        std::cout << "[BossMalachar] Resonance Core transition burst dealt " << burstDamage << " damage to Malachar!\n";

        // Double burst (~1s apart) if Resonance Core was collected/intact
        pendingResonanceBurst = true;
        pendingResonanceBurstTimer = 1.0f;
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

    soulLanceTelegraphMax = foretellActive ? 1.6f : 1.0f;
    soulLanceTelegraphTimer = soulLanceTelegraphMax;
    soulLanceStartPos = getPosition();
    soulLanceTargetPos = getPlayer().getPosition();
    soulLanceCooldown = 10.0f;
}

void BossMalachar::takeDamage(float rawAmount) {
    if (!isAlive()) return;

    // Hollow Bell reflect ward (Phase 1 only)
    if (currentPhase == 1 && reflectWardActive && reflectWardCooldown <= 0.0f) {
        reflectWardCooldown = 8.0f;
        float reflectedDmg = rawAmount * 0.20f;
        getPlayer().takeDamage(reflectedDmg);
        std::cout << "[BossMalachar] Hollow Bell Reflect Ward triggered! Reflected " 
                  << reflectedDmg << " damage back to Serin!\n";
    }

    Enemy::takeDamage(rawAmount);
}

void BossMalachar::onDeath(Chamber* chamber) {
    std::cout << "[BossMalachar] Malachar has been defeated!\n";
    BossChamber* bossChamber = dynamic_cast<BossChamber*>(chamber);
    if (bossChamber) {
        bossChamber->onBossDefeated();
    }
}

void BossMalachar::draw(sf::RenderWindow& window) const {
    // 1. Draw Void Bolt charge glow around Malachar
    if (isVoidBoltCharging) {
        float radius = 35.0f;
        sf::CircleShape aura(radius);
        aura.setOrigin({radius, radius});
        aura.setPosition(getPosition());
        aura.setFillColor(sf::Color(160, 30, 220, 100)); // Glowing purple aura
        aura.setOutlineColor(sf::Color(220, 100, 255, 200));
        aura.setOutlineThickness(3.0f);
        window.draw(aura);
    }

    // 2. Draw Soul Lance charging line & target indicator
    if (isSoulLanceCharging) {
        sf::Vertex line[] = {
            sf::Vertex(getPosition(), sf::Color(255, 30, 30, 220)),
            sf::Vertex(soulLanceTargetPos, sf::Color(255, 100, 0, 255))
        };
        window.draw(line, 2, sf::PrimitiveType::Lines);

        float targetRadius = 15.0f;
        sf::CircleShape targetMarker(targetRadius);
        targetMarker.setOrigin({targetRadius, targetRadius});
        targetMarker.setPosition(soulLanceTargetPos);
        targetMarker.setFillColor(sf::Color(255, 0, 0, 80));
        targetMarker.setOutlineColor(sf::Color::Red);
        targetMarker.setOutlineThickness(2.0f);
        window.draw(targetMarker);
    }

    // 3. Draw active Void Bolts
    for (const auto& bolt : voidBolts) {
        if (!bolt.active) continue;
        float radius = 8.0f;
        sf::CircleShape bShape(radius);
        bShape.setOrigin({radius, radius});
        bShape.setPosition(bolt.position);
        bShape.setFillColor(sf::Color(140, 20, 220, 240));
        bShape.setOutlineColor(sf::Color(240, 180, 255, 255));
        bShape.setOutlineThickness(2.0f);
        window.draw(bShape);
    }

    // 4. Draw base Malachar sprite/shape and health bar
    Enemy::draw(window);
}
