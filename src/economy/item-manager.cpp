#include "item-manager.hpp"
#include "../entities/player.hpp"
#include "../chambers/chamber.hpp"

void ItemManager::update(float dt, Player& player, Chamber& chamber) {
    sf::Vector2f playerPos = player.getPosition();
    for (auto it = items.begin(); it != items.end();) {
        (*it)->update(dt, playerPos);
        if ((*it)->isCollected()) {
            (*it)->onCollect(player, chamber);
            it = items.erase(it);
        } else ++it;
    }
}

void ItemManager::draw(sf::RenderWindow& window) {
    for (const auto& item : items) {
        window.draw(*item);
    }
}

void ItemManager::spawnFragments(sf::Vector2f position, int count) {
    for (int i = 0; i < count; ++i) {
        items.push_back(std::make_unique<EchoFragment>(position, 1.0f));
    }
}

void ItemManager::spawnEnemyFragments(Enemy* enemy, Player& player) {
    if (!enemy) return;
    
    int count = enemy->getFragmentDropCount();
    
    // Ironshell Multiplier: Double drops if killed while Slowed
    if (player.getActiveFormType() == FormType::IRONSHELL && enemy->isSlowed()) {
        count *= 2;
    }
    
    spawnFragments(enemy->getPosition(), count);
}

#include "../chambers/boss-chamber.hpp"
#include "../chambers/gauntlet-chamber.hpp"
#include <random>

void ItemManager::spawnEnemyDrops(Enemy* enemy, Player& player, Chamber& chamber) {
    if (!enemy) return;

    sf::Vector2f pos = enemy->getPosition();
    static std::mt19937 rng(std::random_device{}());

    // 1. Guaranteed Drop Check (e.g. from Boss Malachar's Summoning Burst)
    if (enemy->getGuaranteedDrop().has_value()) {
        const std::string& dropType = *enemy->getGuaranteedDrop();
        if (dropType == "FREEZE_POTION") {
            items.push_back(std::make_unique<FreezePotion>(pos));
        } else if (dropType == "SPEED_POTION") {
            items.push_back(std::make_unique<SpeedPotion>(pos));
        } else if (dropType == "CRITICAL_POTION") {
            items.push_back(std::make_unique<CriticalPotion>(pos));
        } else if (dropType == "ANTIDOTE") {
            items.push_back(std::make_unique<AntidotePotion>(pos));
        } else if (dropType == "HEALTH_POTION") {
            items.push_back(std::make_unique<HealthPotion>(pos));
        } else if (dropType == "RANDOM_NON_FREEZE") {
            // Equal 1/4 distribution among Speed, Critical, Antidote, and Healer for Boss Chamber 2nd add
            std::uniform_int_distribution<int> pickDist(0, 3);
            int pick = pickDist(rng);
            if (pick == 0) {
                items.push_back(std::make_unique<SpeedPotion>(pos));
            } else if (pick == 1) {
                items.push_back(std::make_unique<CriticalPotion>(pos));
            } else if (pick == 2) {
                items.push_back(std::make_unique<AntidotePotion>(pos));
            } else {
                items.push_back(std::make_unique<HealthPotion>(pos));
            }
        }
    } else {
        // 2. Random Drop Rates in Non-Boss Chambers
        bool isBossRoom = (dynamic_cast<BossChamber*>(&chamber) != nullptr);

        if (!isBossRoom) {
            // 25% overall chance that an item drops
            std::uniform_real_distribution<float> dropDist(0.0f, 100.0f);
            if (dropDist(rng) < 25.0f) {
                // Select item with equal distribution among the 5 items (20% each)
                std::uniform_int_distribution<int> itemDist(0, 4);
                int itemType = itemDist(rng);
                if (itemType == 0) {
                    items.push_back(std::make_unique<FreezePotion>(pos));
                } else if (itemType == 1) {
                    items.push_back(std::make_unique<SpeedPotion>(pos));
                } else if (itemType == 2) {
                    items.push_back(std::make_unique<CriticalPotion>(pos));
                } else if (itemType == 3) {
                    items.push_back(std::make_unique<AntidotePotion>(pos));
                } else {
                    items.push_back(std::make_unique<HealthPotion>(pos));
                }
            }
        }
    }

    // 3. Gauntlet Chamber Special: Killing the last enemy drops an additional 3 Healers
    bool isGauntlet = (dynamic_cast<GauntletChamber*>(&chamber) != nullptr);
    if (isGauntlet) {
        if (chamber.getWaveSpawner().isFinished() && chamber.getEnemyCount() <= 1) {
            for (int i = 0; i < 3; ++i) {
                items.push_back(std::make_unique<HealthPotion>(pos));
            }
        }
    }

    // 4. Standard Echo Fragments
    spawnEnemyFragments(enemy, player);
}

void ItemManager::addItem(std::unique_ptr<Item> item) {
    items.push_back(std::move(item));
}
