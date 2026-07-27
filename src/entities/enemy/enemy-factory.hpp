#ifndef ENEMY_FACTORY_HPP
#define ENEMY_FACTORY_HPP

#include <memory>
#include <string>
#include "enemy.hpp"
#include "../../core/enums.hpp"

class EnemyFactory {
public:
    static std::unique_ptr<Enemy> createEnemy(EnemyType type, Player& player);
    static std::unique_ptr<Enemy> createEnemy(const std::string& typeName, Player& player);
    static EnemyType stringToEnemyType(const std::string& typeName);
};

#endif // ENEMY_FACTORY_HPP