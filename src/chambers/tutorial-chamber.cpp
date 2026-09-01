#include "tutorial-chamber.hpp"
#include "map-loader.hpp"
#include "../entities/enemy/dummy-enemy.hpp"
#include "../entities/player.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../global-settings/asset-manager.hpp"
#include "../global-settings/sound-manager.hpp"
#include "../utils/math-utility.hpp"
#include <iostream>
#include <cmath>

TutorialChamber::TutorialChamber(Player& player) : Chamber(player) {
    dropsFragments = false;
    ChamberConfig config = MapLoader::loadChamber("assets/maps/tutorial/tutorial.json");

    if (!config.typeGrid.empty() && !config.levelGrid.empty()) {
        setGrids2D5(config.typeGrid, config.levelGrid, config.bridgeGrid);
    }

    if (config.playerSpawnCell.first >= 0 && config.playerSpawnCell.second >= 0) {
        float spawnX = config.playerSpawnCell.second + 0.5f;
        float spawnY = config.playerSpawnCell.first + 0.5f;
        setPlayerSpawn({spawnX, spawnY});
    }

    float cellSize = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();

    if (config.exitPositionCell.first >= 0 && config.exitPositionCell.second >= 0) {
        float exitX = ox + (config.exitPositionCell.second + 0.5f) * cellSize;
        float exitY = oy + (config.exitPositionCell.first + 0.5f) * cellSize;
        setExitPosition({exitX, exitY});
    } else {
        setExitPosition({ox + 157.0f * cellSize, oy + 13.0f * cellSize});
    }

    if (exitGate) {
        exitGate->setActive(true);
    }

    setupIslandsAndLabels();
    spawnInitialEnemies();
}

TutorialChamber::~TutorialChamber() {
}

void TutorialChamber::setFormGuideCallback(std::function<void()> callback) {
    formGuideCallback = std::move(callback);
}

void TutorialChamber::setupIslandsAndLabels() {
    float cellSize = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();

    floorLabels.clear();

    // 1. Island 1: Movement
    floorLabels.push_back({
        .position = sf::Vector2f(ox + 6.0f * cellSize, oy + 12.0f * cellSize),
        .keyText = "[ W A S D ]",
        .instructionText = "MOVE"
    });

    // 2. Island 2: Sprint
    floorLabels.push_back({
        .position = sf::Vector2f(ox + 28.0f * cellSize, oy + 12.0f * cellSize),
        .keyText = "[ RIGHT CLICK ]",
        .instructionText = "SPRINT DASH"
    });

    // 3. Island 3: Basic Attack
    floorLabels.push_back({
        .position = sf::Vector2f(ox + 50.0f * cellSize, oy + 9.8f * cellSize),
        .keyText = "[ LEFT CLICK ]",
        .instructionText = "ATTACK (0 DMG DUMMY)"
    });

    // 4. Island 4: Form Switching
    floorLabels.push_back({
        .position = sf::Vector2f(ox + 70.0f * cellSize, oy + 12.0f * cellSize),
        .keyText = "[ 1 ]  [ 2 ]  [ 3 ]",
        .instructionText = "SWITCH FORM"
    });

    // 5. Island 5: Ranged Archery
    floorLabels.push_back({
        .position = sf::Vector2f(ox + 90.0f * cellSize, oy + 14.0f * cellSize),
        .keyText = "[ PRESS 2 ] VOIDCASTER",
        .instructionText = "SHOOT ACROSS CHASM"
    });

    // 6. Island 6: Special Abilities
    floorLabels.push_back({
        .position = sf::Vector2f(ox + 110.0f * cellSize, oy + 13.0f * cellSize),
        .keyText = "[ Q ]  [ E ]",
        .instructionText = "SPECIAL ABILITIES"
    });

    // 7. Island 7: Customization
    floorLabels.push_back({
        .position = sf::Vector2f(ox + 134.0f * cellSize, oy + 13.0f * cellSize),
        .keyText = "SETTINGS",
        .instructionText = "REMAP KEYBINDS FREELY"
    });

    // 8. Island 8: Zoom & Exit Portal
    floorLabels.push_back({
        .position = sf::Vector2f(ox + 153.5f * cellSize, oy + 13.0f * cellSize),
        .keyText = "[ MOUSE WHEEL ]",
        .instructionText = "ZOOM VIEW -> PORTAL"
    });
}

void TutorialChamber::spawnInitialEnemies() {
    float cellSize = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();

    // Island 3: Stationary Dummy
    if (!island3DummySpawned) {
        auto dummy3 = std::make_unique<DummyEnemy>(player, "soldier", 40.0f, 0.0f, false);
        dummy3->setPosition({ox + 50.0f * cellSize, oy + 13.0f * cellSize});
        island3Dummy = dummy3.get();
        spawnEnemy(std::move(dummy3));
        island3DummySpawned = true;
    }

    // Island 5: Wandering dummy on isolated island (rows 4..8, cols 87..93)
    if (!isolatedEnemySpawned) {
        auto dummy5 = std::make_unique<DummyEnemy>(player, "sprinter", 30.0f, 2.0f, true);
        dummy5->setPosition({ox + 90.0f * cellSize, oy + 6.0f * cellSize});
        isolatedEnemy = dummy5.get();
        spawnEnemy(std::move(dummy5));
        isolatedEnemySpawned = true;
    }

    // Island 6: 3 Respawning dummy slots
    dummySlots.clear();
    dummySlots.push_back({
        .position = sf::Vector2f(ox + 114.5f * cellSize, oy + 11.0f * cellSize),
        .maxHp = 60.0f,
        .isAlive = false,
        .respawnTimer = 0.0f,
        .currentEnemy = nullptr
    });
    dummySlots.push_back({
        .position = sf::Vector2f(ox + 116.5f * cellSize, oy + 13.0f * cellSize),
        .maxHp = 60.0f,
        .isAlive = false,
        .respawnTimer = 0.0f,
        .currentEnemy = nullptr
    });
    dummySlots.push_back({
        .position = sf::Vector2f(ox + 114.5f * cellSize, oy + 15.0f * cellSize),
        .maxHp = 60.0f,
        .isAlive = false,
        .respawnTimer = 0.0f,
        .currentEnemy = nullptr
    });

    for (auto& slot : dummySlots) {
        auto dummy = std::make_unique<DummyEnemy>(player, "soldier", slot.maxHp, 0.0f, false);
        dummy->setPosition(slot.position);
        slot.currentEnemy = dummy.get();
        slot.isAlive = true;
        slot.respawnTimer = 0.0f;
        spawnEnemy(std::move(dummy));
    }
}

void TutorialChamber::updateIslandTriggers(float dt) {
    float cellSize = SettingManager::getInstance().getCellSize();
    float ox = SettingManager::getInstance().getGridOffsetX();
    float playerCol = (player.getPosition().x - ox) / cellSize;

    // Trigger Island 4 Form Guide Modal Dialog
    if (!formGuideTriggered && playerCol >= 66.0f && playerCol <= 74.5f) {
        formGuideTriggered = true;
        if (formGuideCallback) {
            formGuideCallback();
        }
    }

    // Island 6: Momentum Replenishment & Dummy Respawns
    if (playerCol >= 105.5f && playerCol <= 118.5f) {
        island6Active = true;
        player.setMomentum(100.0f, FormType::WRAITHBLADE);
        player.setMomentum(100.0f, FormType::VOIDCASTER);
        player.setMomentum(100.0f, FormType::IRONSHELL);
    } else {
        island6Active = false;
    }

    // Manage Island 6 Dummy Respawns
    for (auto& slot : dummySlots) {
        if (slot.isAlive) {
            if (!slot.currentEnemy || !slot.currentEnemy->isAlive()) {
                slot.isAlive = false;
                slot.currentEnemy = nullptr;
                slot.respawnTimer = 2.0f; // 2.0s respawn delay
            }
        } else {
            slot.respawnTimer -= dt;
            if (slot.respawnTimer <= 0.0f) {
                auto newDummy = std::make_unique<DummyEnemy>(player, "soldier", slot.maxHp, 0.0f, false);
                newDummy->setPosition(slot.position);
                slot.currentEnemy = newDummy.get();
                slot.isAlive = true;
                spawnEnemy(std::move(newDummy));
            }
        }
    }
}

void TutorialChamber::update(float dt) {
    Chamber::update(dt);
    updateIslandTriggers(dt);
}

void TutorialChamber::renderFloorLabel(sf::RenderWindow& window, const FloorLabel& label, const sf::Font& headerFont, const sf::Font& /*bodyFont*/) const {
    sf::Text keyText(headerFont, label.keyText, 11);
    keyText.setFillColor(sf::Color(255, 215, 75));

    sf::Text instText(headerFont, label.instructionText, 9);
    instText.setFillColor(sf::Color(235, 240, 255));

    sf::FloatRect kb = keyText.getLocalBounds();
    sf::FloatRect ib = instText.getLocalBounds();

    float gap = 8.0f;
    float totalH = kb.size.y + ib.size.y + gap;

    keyText.setPosition({label.position.x - kb.size.x / 2.0f, label.position.y - totalH / 2.0f});
    instText.setPosition({label.position.x - ib.size.x / 2.0f, label.position.y - totalH / 2.0f + kb.size.y + gap});

    window.draw(keyText);
    window.draw(instText);
}

void TutorialChamber::drawForeground(sf::RenderWindow& window) {
    const auto& assets = AssetManager::getInstance();
    const sf::Font& headerFont = assets.getFont("header");
    const sf::Font& bodyFont = assets.getFont("header");

    for (const auto& label : floorLabels) {
        renderFloorLabel(window, label, headerFont, bodyFont);
    }
}

void TutorialChamber::completeChamber() {
    if (!isCompleted && !isFailed) {
        isCompleted = true;
        if (observer) {
            observer->onChamberCompleted();
        }
    }
}
