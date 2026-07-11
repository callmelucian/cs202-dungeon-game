#ifndef ENEMY_FACTORY_HPP
#define ENEMY_FACTORY_HPP

#include <memory>
#include "enemy.hpp"
#include "../../core/enums.hpp"

class EnemyFactory {
public:
    static std::unique_ptr<Enemy> createEnemy(EnemyType type, Player& player);
};

#endif // ENEMY_FACTORY_HPP