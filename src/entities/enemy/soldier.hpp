#ifndef SOLDIER_HPP
#define SOLDIER_HPP

#include "enemy.hpp"

class Soldier : public Enemy {
public:
    Soldier(Player& player);
    ~Soldier() override = default;

    void onDeath(Chamber* chamber = nullptr) override;
};

#endif // SOLDIER_HPP
