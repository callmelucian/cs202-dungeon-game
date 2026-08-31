#ifndef GAUNTLET_CHAMBER_HPP
#define GAUNTLET_CHAMBER_HPP

#include "chamber.hpp"
#include "wave-spawner.hpp"

class GauntletChamber : public Chamber {
private:
    bool cleared = false;

public:
    GauntletChamber(Player& player);
    virtual ~GauntletChamber() = default;

    void update(float dt) override;
};

#endif // GAUNTLET_CHAMBER_HPP
