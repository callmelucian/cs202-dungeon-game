#ifndef TILEMAP_LOADER_HPP
#define TILEMAP_LOADER_HPP

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <SFML/Graphics.hpp>

enum class TileType {
    Void,
    Land,
    Water,
    VerticalBridge,
    HorizontalBridge
};

struct TextureSubRect {
    int x;
    int y;
    int width;
    int height;
};

struct TileQuadInfo {
    TextureSubRect texRect;
    sf::Vector2f renderOffset;
    sf::Vector2f renderSize;
};

struct RenderTile {
    int layer;
    int gridRow;
    int gridCol;
    std::vector<TileQuadInfo> quads;
};

struct TilemapRenderData {
    int width;
    int height;
    std::vector<RenderTile> baseTiles;
    std::vector<RenderTile> cliffTiles;
    std::vector<RenderTile> shadowTiles;
    std::vector<RenderTile> bridgeTiles;
    std::vector<std::vector<int>> legacyGrid;
    std::vector<std::vector<bool>> walkableGrid;
};

class TilemapLoader {
public:
    static TilemapLoader& getInstance();

    // Transforms a 2D raw chamber description into a 2.5D rendered tile matrix
    std::vector<std::vector<int>> loadMap(
        const std::vector<std::vector<std::string>>& typeGrid,
        const std::vector<std::vector<int>>& levelGrid
    );

    // Synthesizes complete 2.5D layout, autotiling, cliffs, shadows, overlays and pathfinding data
    TilemapRenderData synthesizeMap(
        const std::vector<std::vector<std::string>>& typeGrid,
        const std::vector<std::vector<int>>& levelGrid
    );

    TileType parseTileType(const std::string& typeStr) const;

private:
    TilemapLoader();
    ~TilemapLoader() = default;
    TilemapLoader(const TilemapLoader&) = delete;
    TilemapLoader& operator=(const TilemapLoader&) = delete;

    void loadAtlasConfig(const std::string& configPath);

    int tilePicker(
        TileType currentType,
        const std::vector<TileType>& neighbors
    );

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
    CliffAssetEntry waterCliff;
    OverlayAssetEntry landShadowTop;
    OverlayAssetEntry landShadowMiddle;
    OverlayAssetEntry landShadowBottom;
    OverlayAssetEntry verticalBridge;
    OverlayAssetEntry horizontalBridge;

    bool atlasLoaded = false;
};

#endif // TILEMAP_LOADER_HPP
