#include "item-manager.hpp"
#include "../entities/player.hpp"
#include "../chambers/chamber.hpp"
#include <iostream>

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
    if (count > 0) {
        std::cout << "ItemManager: Spawned " << count << " EchoFragments at (" << position.x << ", " << position.y << ")\n";
    }
}

void ItemManager::spawnEnemyFragments(Enemy* enemy, Player& player) {
    if (!enemy) return;
    
    int count = enemy->getFragmentDropCount();
    
    // Ironshell Multiplier: Double drops if killed while Slowed
    if (player.getActiveFormType() == FormType::IRONSHELL && enemy->isSlowed()) {
        count *= 2;
        std::cout << "Ironshell killed a Slowed enemy! Doubling fragments to: " << count << "\n";
    }
    
    spawnFragments(enemy->getPosition(), count);
}

void ItemManager::addItem(std::unique_ptr<Item> item) {
    items.push_back(std::move(item));
}
