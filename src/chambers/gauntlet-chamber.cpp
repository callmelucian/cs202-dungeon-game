#include "gauntlet-chamber.hpp"
#include "../utils/collision-solver.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../entities/enemy/waterlogged-scribe.hpp"
#include "../entities/enemy/bone-sprinter.hpp"
#include "../entities/player.hpp"
#include <iostream>

GauntletChamber::GauntletChamber(Player& player) : Chamber(player), currentWaveIndex(0) {
    dropsFragments = false;
    
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();
    float cell = SettingManager::getInstance().getCellSize();
    
    // Wave 1
    std::vector<std::unique_ptr<Enemy>> wave1;
    auto s1 = std::make_unique<WaterloggedScribe>(player);
    s1->setPosition({ox + 3.5f * cell, oy + 3.5f * cell});
    wave1.push_back(std::move(s1));
    waves.push_back(std::move(wave1));
    
    // Wave 2
    std::vector<std::unique_ptr<Enemy>> wave2;
    auto s2 = std::make_unique<BoneSprinter>(player, false);
    s2->setPosition({ox + 7.5f * cell, oy + 3.5f * cell});
    wave2.push_back(std::move(s2));
    auto s3 = std::make_unique<BoneSprinter>(player, false);
    s3->setPosition({ox + 3.5f * cell, oy + 7.5f * cell});
    wave2.push_back(std::move(s3));
    waves.push_back(std::move(wave2));
}

void GauntletChamber::update(float dt) {
    Chamber::update(dt);
    
    if (enemies.empty() && !isCompleted) {
        if (currentWaveIndex < waves.size()) {
            for (auto& enemyPtr : waves[currentWaveIndex]) {
                spawnEnemy(std::move(enemyPtr));
            }
            std::cout << "Gauntlet: Wave " << currentWaveIndex + 1 << " started!\n";
            currentWaveIndex++;
        }
        else {
            float healAmount = player.getEffectiveStats().maxHp * 0.25f;
            player.heal(healAmount);
            std::cout << "Gauntlet: Chamber cleared! Healed " << healAmount << " HP.\n";
            completeChamber();
        }
    }
}
