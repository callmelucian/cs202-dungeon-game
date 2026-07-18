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
    static MapLoader& getInstance();

    // Prevent copy/move
    MapLoader(const MapLoader&) = delete;
    MapLoader& operator=(const MapLoader&) = delete;

    std::vector<std::vector<int>> loadChamberGrid(int level, int chamberIndex);
    
    ChamberConfig loadChamberConfig(const std::string& filepath);

private:
    MapLoader() = default;
    ~MapLoader() = default;
};

#endif // MAP_LOADER_HPP
