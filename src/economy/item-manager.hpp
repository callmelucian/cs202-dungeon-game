#ifndef ITEM_MANAGER_HPP
#define ITEM_MANAGER_HPP

#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include "item.hpp"
#include "../entities/enemy/enemy.hpp"

class Player;
class Chamber;

class ItemManager {
public:
    void update(float dt, Player& player, Chamber& chamber);
    void draw(sf::RenderWindow& window);

    void spawnFragments(sf::Vector2f position, int count);
    void spawnEnemyFragments(Enemy* enemy, Player& player);
    void addItem(std::unique_ptr<Item> item);

private:
    std::vector<std::unique_ptr<Item>> items;
};

#endif // ITEM_MANAGER_HPP
