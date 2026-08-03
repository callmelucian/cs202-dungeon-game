#ifndef TILE_MANAGER_HPP
#define TILE_MANAGER_HPP

#include <vector>
#include <string>

class TileManager {
public:
    static TileManager& getInstance();

    TileManager(const TileManager&) = delete;
    TileManager& operator=(const TileManager&) = delete;

    bool loadAtlasConfig(const std::string& configPath);

    struct TileAssetEntry {
        std::vector<int> coord;
        std::string position; // "FILLED", "ANY", "TL", "TR", "BL", "BR"
        std::string neighbors;
        int textureVariant = 0;
    };

    struct CliffAssetEntry {
        std::vector<int> coord;
        std::string position;
    };

    struct OverlayAssetEntry {
        std::vector<int> coord;
        std::vector<int> size;
    };

    std::vector<TileAssetEntry> landAssets;
    std::vector<TileAssetEntry> waterAssets;
    CliffAssetEntry hardCliff;
    CliffAssetEntry semiHardCliff;
    CliffAssetEntry softCliff;
    CliffAssetEntry semiSoftCliff;
    CliffAssetEntry waterCliff;
    CliffAssetEntry stairs;
    CliffAssetEntry shadowedStairs;
    OverlayAssetEntry verticalBridge;
    OverlayAssetEntry horizontalBridge;

    bool atlasLoaded = false;

private:
    TileManager() = default;
    ~TileManager() = default;
};

#endif

