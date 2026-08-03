#ifndef WAVE_SPAWNER_HPP
#define WAVE_SPAWNER_HPP

#include <vector>
#include <memory>
#include <string>
#include <SFML/System/Vector2.hpp>
#include "map-loader.hpp"

class Chamber;
class Player;

struct ActiveWave {
    std::string enemyType;
    int totalCount;
    int remainingToSpawn;
    float spawnDelay;
    float timer;
};

class WaveSpawner {
public:
    WaveSpawner() = default;

    void setWaves(const std::vector<WaveConfig>& waveConfigs);
    void update(float dt, Chamber& chamber, Player& player);

    bool isFinished() const;
    int getCurrentWaveIndex() const { return currentWaveIndex; }
    int getTotalWaves() const { return static_cast<int>(waves.size()); }
    bool isWaveInProgress() const { return currentWaveIndex < static_cast<int>(waves.size()); }

private:
    std::vector<ActiveWave> waves;
    int currentWaveIndex = 0;
};

#endif // WAVE_SPAWNER_HPP
