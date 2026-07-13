#ifndef MAP_LOADER_HPP
#define MAP_LOADER_HPP

#include <vector>

class MapLoader {
public:
    static MapLoader& getInstance();

    // Prevent copy/move
    MapLoader(const MapLoader&) = delete;
    MapLoader& operator=(const MapLoader&) = delete;

    std::vector<std::vector<int>> loadChamberGrid(int level, int chamberIndex);

private:
    MapLoader() = default;
    ~MapLoader() = default;
};

#endif // MAP_LOADER_HPP
