#ifndef GAUNTLET_CHAMBER_HPP
#define GAUNTLET_CHAMBER_HPP

#include "chamber.hpp"
#include <vector>
#include <memory>

class GauntletChamber : public Chamber {
public:
    GauntletChamber(Player& player);
    virtual ~GauntletChamber() = default;

    void update(float dt) override;

private:
    std::vector<std::vector<std::unique_ptr<Enemy>>> waves;
    int currentWaveIndex;
};

#endif // GAUNTLET_CHAMBER_HPP
