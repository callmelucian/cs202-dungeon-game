#ifndef MAP_LOADER_HPP
#define MAP_LOADER_HPP

#include <vector>

#include <string>
#include <vector>

struct WaveConfig {
    std::string enemyType;
    int count;
    float spawnDelay;
};

struct ChamberConfig {
    std::string chamberType;
    int width;
    int height;
    std::vector<std::vector<int>> grid;
    std::vector<WaveConfig> waves;
};

class MapLoader {
public:
    static std::vector<std::vector<int>> loadChamberGrid(int level, int chamberIndex);
    static ChamberConfig loadChamber(const std::string& filepath);
    static std::vector<WaveConfig> loadWaves(const std::string& filepath);

private:
    MapLoader() = delete; // Static class
};

#endif // MAP_LOADER_HPP
