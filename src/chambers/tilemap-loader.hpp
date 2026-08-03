#ifndef TILEMAP_LOADER_HPP
#define TILEMAP_LOADER_HPP

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <SFML/Graphics.hpp>

#include "renderable-tilemap.hpp"

#include "../global-settings/tile-manager.hpp"

class TilemapLoader {
public:
    static TilemapLoader& getInstance();


    // Synthesizes complete 2.5D layout, autotiling, cliffs, shadows, overlays and pathfinding data
    TilemapRenderData synthesizeMap(
        const std::vector<std::vector<std::string>>& typeGrid,
        const std::vector<std::vector<int>>& levelGrid
    );

    // Creates a RenderableTileMap from the grids
    RenderableTileMap createRenderableMap(
        const std::vector<std::vector<std::string>>& typeGrid,
        const std::vector<std::vector<int>>& levelGrid,
        float cellSize, float offsetX, float offsetY
    );

    TileType parseTileType(const std::string& typeStr) const;

private:
    TilemapLoader();
    ~TilemapLoader() = default;
    TilemapLoader(const TilemapLoader&) = delete;
    TilemapLoader& operator=(const TilemapLoader&) = delete;

    int tilePicker(
        TileType currentType,
        const std::vector<TileType>& neighbors
    );
};

#endif // TILEMAP_LOADER_HPP
