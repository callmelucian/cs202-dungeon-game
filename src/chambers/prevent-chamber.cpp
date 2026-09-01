#include "prevent-chamber.hpp"
#include "../entities/player.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../global-settings/sound-manager.hpp"
#include "../ui/graphics/particle-system.hpp"
#include "../utils/math-utility.hpp"
#include "../entities/enemy/enemy-state.hpp"
#include "../ui/widgets/floating-text-manager.hpp"
#include "../core/game.hpp"
#include <iostream>

PreventChamber::PreventChamber(Player& player, std::optional<EchoType> echoType)
    : Chamber(player), associatedEcho(echoType) {
    
    float cellSize = SettingManager::getInstance().getCellSize();
    exitShape.setSize(sf::Vector2f(cellSize, cellSize));
    exitShape.setOrigin(sf::Vector2f(cellSize / 2.0f, cellSize / 2.0f));
    exitShape.setFillColor(sf::Color(255, 100, 100, 150));
    exitShape.setOutlineColor(sf::Color::Red);
    exitShape.setOutlineThickness(2.0f);
}

void PreventChamber::setExitPosition(const sf::Vector2f& pos) {
    Chamber::setExitPosition(pos);
    exitShape.setPosition(pos);
}

void PreventChamber::update(float dt) {
    Chamber::update(dt);
    
    float cellSize = SettingManager::getInstance().getCellSize();
    for (auto& enemy : enemies) {
        // Check if carrier reached exit
        if (enemy->getIsRealCarrier() && enemy->isAlive()) {
            if (Math::distance(enemy->getPosition(), exitPosition) <= 0.5f * cellSize) {
                std::cout << "Carrier reached the exit! Echo STOLEN.\n";
                SoundManager::getInstance().playSound("enemy-hit");
                ParticleSystem::getInstance().emitBurst(exitPosition, 30, sf::Color(255, 50, 50, 200), 40.0f, 120.0f, 0.3f, 0.8f, 5.0f);
                RunState& runState = Game::getInstance().getRunState();
                if (associatedEcho.has_value()) {
                    runState.echoOutcomes[*associatedEcho] = EchoOutcome::STOLEN;
                    runState.echoPowers[*associatedEcho] = 0.0f;
                    runState.echoesStolen++;

                    std::string echoName = "Echo";
                    if (*associatedEcho == EchoType::CLARITY_SHARD) echoName = "Clarity Shard";
                    else if (*associatedEcho == EchoType::MARROW) echoName = "Marrow Echo";
                    else if (*associatedEcho == EchoType::HOLLOW_BELL) echoName = "Hollow Bell";
                    else if (*associatedEcho == EchoType::RESONANCE_CORE) echoName = "Resonance Core";
                    else if (*associatedEcho == EchoType::OBSIDIAN_KEY) echoName = "Obsidian Key";

                    sf::Vector2f notifyPos = exitPosition + sf::Vector2f(0.0f, -50.0f);
                    UI::FloatingTextManager::getInstance().spawnEchoStolen(notifyPos, echoName);
                }
                
                // Carrier escapes through exit gate — no item/fragment drops
                enemy->setHasEscaped(true);
                enemy->setHp(0.0f); // Escaped carrier removed, chamber continues!
            }
        }
    }

    if (waveSpawner.isFinished() && enemies.empty() && !isFailed && !wavesCleared) {
        wavesCleared = true;
        RunState& runState = Game::getInstance().getRunState();
        if (associatedEcho.has_value() && runState.echoOutcomes[*associatedEcho] != EchoOutcome::STOLEN) {
            runState.echoOutcomes[*associatedEcho] = EchoOutcome::COLLECTED;
            runState.echoPowers[*associatedEcho] = 100.0f;
            runState.syncEchoModifiers();

            std::string echoName = "Echo";
            if (*associatedEcho == EchoType::CLARITY_SHARD) echoName = "Clarity Shard";
            else if (*associatedEcho == EchoType::MARROW) echoName = "Marrow Echo";
            else if (*associatedEcho == EchoType::HOLLOW_BELL) echoName = "Hollow Bell";
            else if (*associatedEcho == EchoType::RESONANCE_CORE) echoName = "Resonance Core";
            else if (*associatedEcho == EchoType::OBSIDIAN_KEY) echoName = "Obsidian Key";

            sf::Vector2f notifyPos = player.getPosition() + sf::Vector2f(0.0f, -50.0f);
            UI::FloatingTextManager::getInstance().spawnEchoCollected(notifyPos, echoName, 100.0f);
            std::cout << "PreventChamber: All enemies defeated! Echo safely preserved and COLLECTED.\n";
        }
        
        if (exitGate) {
            exitGate->setActive(true);
        }
        sf::Vector2f gateNotifyPos = (exitPosition.x >= 0.0f) ? (exitPosition + sf::Vector2f(0.0f, -40.0f)) : (player.getPosition() + sf::Vector2f(0.0f, -30.0f));
        UI::FloatingTextManager::getInstance().spawnText(gateNotifyPos, "EXIT GATE OPENED", sf::Color(100, 255, 200), 12, "header", 3.0f);
    }
}

void PreventChamber::drawBackground(sf::RenderWindow& window) {
    if (!wavesCleared) {
        // Draw carrier exit zone
        window.draw(exitShape);
    }
}

void PreventChamber::onEnemyHit(Enemy* enemy, bool lethal) {
    Chamber::onEnemyHit(enemy, lethal);
    // onCarrierHit(enemy, lethal);
    if (!lethal && enemy->getIsRealCarrier()) {
        // Trigger 0.5s stagger on real carrier
        enemy->changeState(std::make_unique<StaggeredState>(0.5f, std::make_unique<ChasingState>()));
        std::cout << "Real Carrier hit! Staggered.\n";
    }
}
