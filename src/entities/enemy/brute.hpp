#ifndef BRUTE_HPP
#define BRUTE_HPP

#include "enemy.hpp"

class Brute : public Enemy {
public:
    Brute(Player& player);
    ~Brute() override = default;

    void onDeath(Chamber* chamber = nullptr) override;
};

#endif // BRUTE_HPP
