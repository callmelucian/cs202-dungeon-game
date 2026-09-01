#include "protect-chamber.hpp"
#include "../entities/player.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../utils/math-utility.hpp"
#include "../utils/pathfinder.hpp"
#include "../global-settings/sound-manager.hpp"
#include "../ui/graphics/particle-system.hpp"
#include "../ui/widgets/floating-text-manager.hpp"
#include "../core/game.hpp"

#include "../global-settings/asset-manager.hpp"
#include "../ui/graphics/aura-renderer.hpp"

ProtectChamber::ProtectChamber(Player& player, const std::string& echoName, float requiredTime, EchoType echoType)
    : Chamber(player), collectionTimer(0.0f), requiredCollectionTime(requiredTime), isCollected(false), associatedEcho(echoType),
      echoSprite(AssetManager::getInstance().getTexture("items"), sf::IntRect({72, 48}, {24, 24})),
      floatTimer(0.0f), sparkleTimer(0.0f) {
    echo = std::make_unique<Echo>(echoName, 50.0f); // Starts at 50% power

    // echo-fragment-3 sprite setup (1x scale, floating)
    echoSprite.setOrigin(sf::Vector2f(12.0f, 12.0f));
    echoSprite.setScale(sf::Vector2f(1.0f, 1.0f));

    float cellSize = SettingManager::getInstance().getCellSize();
    auraRadius = 2.1f * cellSize;
}

void ProtectChamber::setEchoPosition(sf::Vector2f pos) {
    echoPosition = pos;
    echoSprite.setPosition(pos);
    // Prime the bar's sprite positions immediately so they're correct on the
    // first draw (during the chamber intro, before update() has ever run).
    collectorTimerBar.setPosition(echoPosition + sf::Vector2f(-29.0f, -38.0f));
}

void ProtectChamber::update(float dt) {
    Chamber::update(dt);

    floatTimer += dt;
    sparkleTimer += dt;

    float floatOffsetY = std::sin(floatTimer * 2.4f) * 5.0f;
    echoSprite.setPosition(echoPosition + sf::Vector2f(0.0f, floatOffsetY));

    if (!isCollected && sparkleTimer >= 0.2f) {
        sparkleTimer = 0.0f;
        ParticleSystem::getInstance().emitSparkle(echoPosition + sf::Vector2f(0.0f, floatOffsetY), 1, sf::Color(210, 140, 255, 180), 16.0f);
    }

    // 2. Update Echo collection
    if (!isCollected) {
        float cellSize = SettingManager::getInstance().getCellSize();
        float distToPlayer = Math::distance(player.getPosition(), echoPosition);
        
        if (distToPlayer <= auraRadius) {
            collectionTimer += dt;
            if (collectionTimer >= requiredCollectionTime) {
                isCollected = true;
                SoundManager::getInstance().playSound("echo-collect");
                ParticleSystem::getInstance().emitGlow(echoPosition, 40, sf::Color(255, 255, 200, 200), 50.0f);

                RunState& runState = Game::getInstance().getRunState();
                if (!isReliquaryDecoy) {
                    runState.echoOutcomes[associatedEcho] = EchoOutcome::COLLECTED;
                    runState.echoPowers[associatedEcho] = echo->getPower();
                    runState.syncEchoModifiers();

                    std::string echoName = "Echo";
                    if (associatedEcho == EchoType::CLARITY_SHARD) echoName = "Clarity Shard";
                    else if (associatedEcho == EchoType::MARROW) echoName = "Marrow Echo";
                    else if (associatedEcho == EchoType::HOLLOW_BELL) echoName = "Hollow Bell";
                    else if (associatedEcho == EchoType::RESONANCE_CORE) echoName = "Resonance Core";
                    else if (associatedEcho == EchoType::OBSIDIAN_KEY) echoName = "Obsidian Key";

                    sf::Vector2f notifyPos = player.getPosition() + sf::Vector2f(0.0f, -50.0f);
                    UI::FloatingTextManager::getInstance().spawnEchoCollected(notifyPos, echoName, echo->getPower());
                } else {
                    runState.hasDecoyReliquaryBuff = true;
                    float buffAmount = player.getEffectiveStats().maxHp * 0.20f;
                    player.heal(buffAmount);
                    sf::Vector2f notifyPos = player.getPosition() + sf::Vector2f(0.0f, -50.0f);
                    UI::FloatingTextManager::getInstance().spawnText(notifyPos, "+ SACRED RELIQUARY DEFENDED! (+20% MAX HP)", sf::Color(80, 240, 140), 10, "header", 2.2f);
                }
                
                if (exitGate) {
                    exitGate->setActive(true);
                }
                sf::Vector2f gateNotifyPos = (exitPosition.x >= 0.0f) ? (exitPosition + sf::Vector2f(0.0f, -40.0f)) : (player.getPosition() + sf::Vector2f(0.0f, -30.0f));
                UI::FloatingTextManager::getInstance().spawnText(gateNotifyPos, "EXIT GATE OPENED", sf::Color(100, 255, 200), 12, "header", 3.0f);
            }
        } else {
            // Decay progress if player leaves the circle
            collectionTimer = std::max(0.0f, collectionTimer - dt * 0.5f);
        }

        // 3. Check enemy attacks on Echo
        for (Enemy* enemy : getEnemiesRaw()) {
            if (!enemy || !enemy->isAlive()) continue;
            float distToEcho = Math::distance(enemy->getPosition(), echoPosition);
            if (distToEcho <= 1.2f * cellSize) {
                if (enemy->getAttackCooldown() <= 0.0f) {
                    onEchoHit(enemy->getEffectiveStats().damage);
                    enemy->setAttackCooldown(2.0f);
                }
            }
        }

        // Update Echo collection timer bar (EnemyHealthBar UI)
        collectorTimerBar.setHealth(collectionTimer, requiredCollectionTime);
        collectorTimerBar.setPosition(echoPosition + sf::Vector2f(-29.0f, -38.0f));
        collectorTimerBar.update(dt);
    }
}

void ProtectChamber::drawBackground(sf::RenderWindow& window) {
    if (!isCollected) {
        AuraRenderer::getInstance().drawAura(window, echoPosition, auraRadius, sf::Color(190, 80, 255), sf::Color(70, 15, 120), 0.75f, 1.2f);
        window.draw(echoSprite);
    }
}

void ProtectChamber::drawForeground(sf::RenderWindow& window) {
    if (!isCollected) {
        collectorTimerBar.draw(window);
    }
}

#include "../entities/enemy/enemy-factory.hpp"

int ProtectChamber::processPlayerAttack(const Hitbox& hitbox) {
    int totalHits = Chamber::processPlayerAttack(hitbox);
    
    if (isNoiseHall && noiseStalkerCount < 12) {
        noiseStalkerCount++;
        auto stalker = EnemyFactory::createEnemy("HUSHED_STALKER", player);
        if (stalker) {
            float cell = SettingManager::getInstance().getCellSize();
            float ox = SettingManager::getInstance().getGridOffsetX();
            float oy = SettingManager::getInstance().getGridOffsetY();
            stalker->setPosition({ox + 3.0f * cell, oy + 3.0f * cell});
            spawnEnemy(std::move(stalker));
        }
    }
    return totalHits;
}

void ProtectChamber::onEnemyHit(Enemy* enemy, bool lethal) {
    if (lethal && enemy && isNoiseHall) {
        if (enemy->isSlowed()) {
            itemManager.spawnFragments(enemy->getPosition(), 2);
        }
    }
    Chamber::onEnemyHit(enemy, lethal);
}

void ProtectChamber::onEchoHit(float rawDamage) {
    if (!echo || isCollected) return;
    if (checkIronshellRedirect()) {
        player.takeDamage(rawDamage);
    } else {
        // 8% penalty per hit on Echo
        echo->takeDamage(8.0f);
    }
}

void ProtectChamber::onFragmentCollected(float value) {
    if (isCollected) return; // Echo is already fully stabilized
    
    // Mid-collection is when the capture progress has started (timer > 0)
    bool midCollection = (collectionTimer > 0.0f);
    
    // Formula: +5.0% pre-collection, +2.5% mid-collection per fragment unit
    float powerGain = midCollection ? (value * 2.5f) : (value * 5.0f);
    
    if (echo) {
        echo->addPower(powerGain);
    }
}

bool ProtectChamber::checkIronshellRedirect() {
    if (player.getActiveFormType() == FormType::IRONSHELL) {
        float cellSize = SettingManager::getInstance().getCellSize();
        float dist = Math::distance(player.getPosition(), echoPosition);
        if (dist <= 1.0f * cellSize) {
            return true;
        }
    }
    return false;
}
