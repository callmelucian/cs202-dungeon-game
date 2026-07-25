#ifndef WATERLOGGED_SCRIBE_HPP
#define WATERLOGGED_SCRIBE_HPP

#include "enemy.hpp"

class WaterloggedScribe : public Enemy {
public:
    WaterloggedScribe(Player& player);
    
    void onDeath(Chamber* chamber = nullptr) override;
};

#endif // WATERLOGGED_SCRIBE_HPP