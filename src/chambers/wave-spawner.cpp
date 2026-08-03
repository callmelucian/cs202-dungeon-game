#include "wave-spawner.hpp"
#include "chamber.hpp"
#include "../entities/enemy/enemy-factory.hpp"
#include "../global-settings/setting-manager.hpp"
#include <iostream>
#include <cstdlib>

void WaveSpawner::setWaves(const std::vector<WaveConfig>& waveConfigs) {
    waves.clear();
    currentWaveIndex = 0;
    for (const auto& config : waveConfigs) {
        ActiveWave wave;
        wave.enemyType = config.enemyType;
        wave.totalCount = config.count;
        wave.remainingToSpawn = config.count;
        wave.spawnDelay = config.spawnDelay;
        wave.timer = 0.0f; // first spawn happens immediately when wave starts
        waves.push_back(wave);
    }
}

void WaveSpawner::update(float dt, Chamber& chamber, Player& player) {
    if (isFinished()) return;

    ActiveWave& currentWave = waves[currentWaveIndex];

    // If there are still enemies to spawn in this wave
    if (currentWave.remainingToSpawn > 0) {
        currentWave.timer -= dt;
        if (currentWave.timer <= 0.0f) {
            auto enemy = EnemyFactory::createEnemy(currentWave.enemyType, player);
            if (enemy) {
                float cell = SettingManager::getInstance().getCellSize();
                float ox = SettingManager::getInstance().getGridOffsetX();
                float oy = SettingManager::getInstance().getGridOffsetY();

                // Collect all walkable ground cells ("L") to ensure enemies never spawn in obstacles
                const auto& grid = chamber.getTypeGrid();
                std::vector<sf::Vector2f> walkablePositions;
                if (!grid.empty()) {
                    int rows = static_cast<int>(grid.size());
                    int cols = static_cast<int>(grid[0].size());
                    for (int r = 1; r < rows - 1; ++r) {
                        for (int c = 1; c < cols - 1; ++c) {
                            if (grid[r][c] == "L") { // "L" is walkable ground floor
                                walkablePositions.push_back({ox + (c + 0.5f) * cell, oy + (r + 0.5f) * cell});
                            }
                        }
                    }
                }

                if (!walkablePositions.empty()) {
                    sf::Vector2f spawnPos = walkablePositions[std::rand() % walkablePositions.size()];
                    enemy->setPosition(spawnPos);
                } else {
                    // Fallback
                    float spawnX = ox + (3.0f + static_cast<float>(std::rand() % 14)) * cell;
                    float spawnY = oy + (3.0f + static_cast<float>(std::rand() % 14)) * cell;
                    enemy->setPosition({spawnX, spawnY});
                }

                chamber.spawnEnemy(std::move(enemy));
                currentWave.remainingToSpawn--;
                currentWave.timer = currentWave.spawnDelay;
            }
        }
    } 
    // If all enemies of this wave have spawned and no active enemies remain in the chamber, advance wave
    else if (chamber.getEnemiesRaw().empty()) {
        std::cout << "WaveSpawner: Wave " << (currentWaveIndex + 1) << " cleared!\n";
        currentWaveIndex++;
    }
}

bool WaveSpawner::isFinished() const {
    return currentWaveIndex >= static_cast<int>(waves.size());
}
