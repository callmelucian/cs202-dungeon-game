#ifndef MAP_LOADER_HPP
#define MAP_LOADER_HPP

#include <vector>

#include <string>
#include <vector>
#include <map>

struct WaveConfig {
    std::string enemyType;
    int count;
    float spawnDelay;
};

struct ChamberConfig {
    std::string chamberType;
    int width;
    int height;
    float playerSpawnX = -1.0f;
    float playerSpawnY = -1.0f;
    std::pair<int, int> playerSpawnCell = {-1, -1};
    float exitPositionX = -1.0f;
    float exitPositionY = -1.0f;
    std::pair<int, int> exitPositionCell = {-1, -1};
    std::vector<std::vector<std::string>> typeGrid;
    std::vector<std::vector<int>> levelGrid;
    std::vector<std::vector<std::string>> bridgeGrid;
    std::vector<WaveConfig> waves;
    std::map<int, std::string> phaseMaps;
};

struct CampaignLevelConfig {
    int levelIndex;
    std::string name;
    std::vector<std::string> chambers;
};

struct CampaignConfig {
    std::vector<CampaignLevelConfig> levels;
};

class MapLoader {
public:
    static std::string getChamberFilepath(int level, int chamberIndex);
    static CampaignConfig loadCampaign(const std::string& filepath);
    static ChamberConfig loadChamber(const std::string& filepath);
    static std::vector<WaveConfig> loadWaves(const std::string& filepath);

private:
    MapLoader() = delete; // Static class
};

#endif // MAP_LOADER_HPP
