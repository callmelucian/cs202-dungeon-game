#ifndef SPRINTER_HPP
#define SPRINTER_HPP

#include "enemy.hpp"

class Sprinter : public Enemy {
private:
    bool isCarrier;

public:
    Sprinter(Player& player, bool isCarrier = false);
    ~Sprinter() override = default;

    void onDeath(Chamber* chamber = nullptr) override;
};

#endif // SPRINTER_HPP
