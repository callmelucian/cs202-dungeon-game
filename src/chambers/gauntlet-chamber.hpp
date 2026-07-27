#ifndef GAUNTLET_CHAMBER_HPP
#define GAUNTLET_CHAMBER_HPP

#include "chamber.hpp"
#include "wave-spawner.hpp"

class GauntletChamber : public Chamber {
public:
    GauntletChamber(Player& player);
    virtual ~GauntletChamber() = default;

    void setWaves(const std::vector<WaveConfig>& configs);
    void update(float dt) override;

private:
    WaveSpawner waveSpawner;
};

#endif // GAUNTLET_CHAMBER_HPP
