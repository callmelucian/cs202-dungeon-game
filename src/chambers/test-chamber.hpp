#ifndef TEST_CHAMBER_HPP
#define TEST_CHAMBER_HPP

#include "chamber.hpp"

class TestChamber : public Chamber {
public:
    TestChamber(Player& player);
    
    void update(float dt) override;
    void onEnemyHit(Enemy* enemy, bool lethal) override;
};

#endif // TEST_CHAMBER_HPP
