#include "test-chamber.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../entities/enemy/sprinter.hpp"
#include "../entities/enemy/soldier.hpp"
#include "../entities/enemy/brute.hpp"
#include "../entities/enemy/enemy-factory.hpp"
#include "../entities/effects/slowed-effect.hpp"
#include "../economy/item.hpp"
#include "../entities/player.hpp"

TestChamber::TestChamber(Player& player) : Chamber(player) {
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();
    float cell = SettingManager::getInstance().getCellSize();

    auto sprinter = std::make_unique<Sprinter>(player, false);
    sprinter->setPosition({ox + 3.5f * cell, oy + 3.5f * cell});
    spawnEnemy(std::move(sprinter));

    auto soldier = std::make_unique<Soldier>(player);
    soldier->setPosition({ox + 7.5f * cell, oy + 3.5f * cell});
    spawnEnemy(std::move(soldier));

    auto brute = std::make_unique<Brute>(player);
    brute->setPosition({ox + 5.5f * cell, oy + 7.5f * cell});
    spawnEnemy(std::move(brute));

    auto carrierSprinter = std::make_unique<Sprinter>(player, true);
    carrierSprinter->setPosition({300.f, 400.f});
    spawnEnemy(std::move(carrierSprinter));
    
    // Spawn a dummy EchoFragment for testing physics and magnet
    itemManager.addItem(std::make_unique<EchoFragment>(sf::Vector2f(200.f, 200.f), 10.0f));
}

void TestChamber::update(float dt) {
    Chamber::update(dt);
    
    if (enemies.empty()) {
        completeChamber();
    }
}
