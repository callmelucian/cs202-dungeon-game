#include "tile-map-generator.hpp"
#include "../global-settings/tile-manager.hpp"
#include "../global-settings/asset-manager.hpp"
#include <iostream>
#include <cstdlib>

RenderableTileMap::RenderableTileMap() 
    : vertices(sf::PrimitiveType::Triangles), texture(nullptr) {}

void RenderableTileMap::load(const sf::Texture* tex, const std::vector<TileData>& tiles, float cellSize) {
    texture = tex;
    
    if (tiles.empty()) return;
    
    vertices.resize(tiles.size() * 6);
    
    for (size_t i = 0; i < tiles.size(); ++i) {
        int x = tiles[i].x;
        int y = tiles[i].y;
        int tileId = tiles[i].tileId;
        
        sf::IntRect texRect = TileManager::getInstance().getTileRect(tileId);
        
        // Generate 6 vertices for a quad (2 triangles)
        sf::Vertex* quad = &vertices[i * 6];
        
        // Triangle 1
        quad[0].position = sf::Vector2f(x * cellSize, y * cellSize);
        quad[1].position = sf::Vector2f((x + 1) * cellSize, y * cellSize);
        quad[2].position = sf::Vector2f(x * cellSize, (y + 1) * cellSize);
        
        // Triangle 2
        quad[3].position = sf::Vector2f(x * cellSize, (y + 1) * cellSize);
        quad[4].position = sf::Vector2f((x + 1) * cellSize, y * cellSize);
        quad[5].position = sf::Vector2f((x + 1) * cellSize, (y + 1) * cellSize);
        
        // Texture coordinates
        float tu = static_cast<float>(texRect.position.x);
        float tv = static_cast<float>(texRect.position.y);
        float tw = static_cast<float>(texRect.size.x);
        float th = static_cast<float>(texRect.size.y);
        
        quad[0].texCoords = sf::Vector2f(tu, tv);
        quad[1].texCoords = sf::Vector2f(tu + tw, tv);
        quad[2].texCoords = sf::Vector2f(tu, tv + th);
        
        quad[3].texCoords = sf::Vector2f(tu, tv + th);
        quad[4].texCoords = sf::Vector2f(tu + tw, tv);
        quad[5].texCoords = sf::Vector2f(tu + tw, tv + th);
    }
}

void RenderableTileMap::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    states.texture = texture;
    target.draw(vertices, states);
}

// 9-slice mapping logic for walls and lakes
static int getTileIdFor9Slice(const std::vector<std::vector<int>>& grid, int x, int y, int type, const std::string& prefix, bool invert = false) {
    bool wallUp    = (y == 0                      || grid[y - 1][x] == type);
    bool wallDown  = (y == (int)grid.size() - 1   || grid[y + 1][x] == type);
    bool wallLeft  = (x == 0                      || grid[y][x - 1] == type);
    bool wallRight = (x == (int)grid[y].size() - 1 || grid[y][x + 1] == type);

    bool u = invert ? wallDown : wallUp;
    bool d = invert ? wallUp   : wallDown;
    bool l = invert ? wallRight : wallLeft;
    bool r = invert ? wallLeft  : wallRight;

    std::string suffix;
    
    // Explicit corner handling for outer bounds (since they are 1-thick walls surrounded by out-of-bounds walls)
    if (invert && x == 0                       && y == 0)                      suffix = "upper-left";
    else if (invert && x == (int)grid[y].size()-1 && y == 0)                   suffix = "upper-right";
    else if (invert && x == 0                  && y == (int)grid.size()-1)     suffix = "lower-left";
    else if (invert && x == (int)grid[y].size()-1 && y == (int)grid.size()-1) suffix = "lower-right";
    else if (!u && !l) suffix = "upper-left";
    else if (!u && !r) suffix = "upper-right";
    else if (!d && !l) suffix = "lower-left";
    else if (!d && !r) suffix = "lower-right";
    else if (!u) suffix = "upper";
    else if (!d) suffix = "lower";
    else if (!l) suffix = "left";
    else if (!r) suffix = "right";
    else suffix = "center";

    int id = TileManager::getInstance().getTileId(prefix + "-" + suffix);
    if (id == -1 && suffix != "center") {
        id = TileManager::getInstance().getTileId(prefix + "-center");
    }
    return id;
}

/**
 * @brief Determines which floor tile to render for a ground-level cell (type 0).
 *
 * For each of the four edges of the cell, an edge texture is applied when the
 * neighbour in that direction is either:
 *   - Out of map bounds (map boundary), or
 *   - Part of the elevated island group (types 4, 5, 6).
 *
 * Special rules:
 *   - Top edge: also triggers when the neighbour above is type 6 (wall-front /
 *     cliff face), since ground just below a cliff face needs a top-edge trim.
 *     (Type 6 is already in the elevated group, so this is naturally handled.)
 *   - Bottom edge: suppressed when the neighbour below is type 5 (stairs),
 *     because stair tiles visually merge with the ground below them.
 *
 * Returns the TileManager tile-ID for the chosen "tile-*" variant.
 */
static int getFloorTileId(const std::vector<std::vector<int>>& grid, int x, int y) {
    int rows = static_cast<int>(grid.size());
    int cols = static_cast<int>(grid[0].size());

    // Returns true when the neighbour (nx, ny) is out of bounds (map boundary)
    auto isBoundary = [&](int nx, int ny) -> bool {
        return nx < 0 || nx >= cols || ny < 0 || ny >= rows;
    };

    bool topEdge = false;
    if (y - 1 < 0) {
        topEdge = true; // Map boundary
    } else if (y == 1) {
        topEdge = true; // Row lying under the top cliff row (which is at y == 0)
    } else {
        topEdge = (grid[y - 1][x] == 6); // Only if directly under a cliff tile (wall-front)
    }

    bool bottomEdge = isBoundary(x, y + 1);
    bool leftEdge   = isBoundary(x - 1, y);
    bool rightEdge  = isBoundary(x + 1, y);

    // Select tile name from edge combination
    std::string suffix;
    if      (topEdge && bottomEdge && leftEdge && rightEdge) suffix = "surrounded";
    else if (topEdge && bottomEdge && leftEdge)  suffix = "upper-lower-left";
    else if (topEdge && bottomEdge && rightEdge) suffix = "upper-lower-right";
    else if (topEdge && leftEdge   && rightEdge) suffix = "upper-left-right";
    else if (bottomEdge && leftEdge && rightEdge) suffix = "lower-left-right";
    else if (topEdge    && leftEdge)  suffix = "upper-left";
    else if (topEdge    && rightEdge) suffix = "upper-right";
    else if (bottomEdge && leftEdge)  suffix = "lower-left";
    else if (bottomEdge && rightEdge) suffix = "lower-right";
    else if (topEdge)    suffix = "upper";
    else if (bottomEdge) suffix = "lower";
    else if (leftEdge)   suffix = "left";
    else if (rightEdge)  suffix = "right";
    else {
        // Interior floor — randomise between plain center and stone variant
        if (std::rand() % 100 < 5) return TileManager::getInstance().getTileId("tile-center-with-stone");
        return TileManager::getInstance().getTileId("tile-center");
    }

    int id = TileManager::getInstance().getTileId("tile-" + suffix);
    if (id == -1) id = TileManager::getInstance().getTileId("tile-center");
    return id;
}

RenderableTileMap TileMapGenerator::generate(const std::vector<std::vector<int>>& semanticGrid, float cellSize, float offsetX, float offsetY) {
    if (semanticGrid.empty() || semanticGrid[0].empty()) return RenderableTileMap();
    
    int width  = static_cast<int>(semanticGrid[0].size());
    int height = static_cast<int>(semanticGrid.size());
    
    std::vector<TileData> tiles;
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int type = semanticGrid[y][x];
            int baseTileId = -1;
            
            if (type == 1) { // Wall
                baseTileId = getTileIdFor9Slice(semanticGrid, x, y, 1, "tile", true);
            } else if (type == 2) { // Lake
                baseTileId = getTileIdFor9Slice(semanticGrid, x, y, 2, "lake");
            } else if (type == 4) { // Elevated floor
                // Types 4, 5, 6 are all part of the elevated island group
                auto isElevated = [&](int nx, int ny) {
                    if (nx < 0 || nx >= width || ny < 0 || ny >= height) return false;
                    int t = semanticGrid[ny][nx];
                    return t == 4 || t == 5 || t == 6;
                };
                bool u = isElevated(x, y - 1);
                bool d = isElevated(x, y + 1);
                bool l = isElevated(x - 1, y);
                bool r = isElevated(x + 1, y);
                
                std::string suffix;
                if      (!u && !l) suffix = "upper-left";
                else if (!u && !r) suffix = "upper-right";
                else if (!d && !l) suffix = "lower-left";
                else if (!d && !r) suffix = "lower-right";
                else if (!u) suffix = "upper";
                else if (!d) suffix = "lower";
                else if (!l) suffix = "left";
                else if (!r) suffix = "right";
                else suffix = "center";
                baseTileId = TileManager::getInstance().getTileId("tile-" + suffix);
            } else if (type == 5) { // Stairs
                baseTileId = TileManager::getInstance().getTileId("stair");
            } else if (type == 6) { // Cliff face (wall-front row of elevated island)
                baseTileId = TileManager::getInstance().getTileId("wall-front");
            } else { // Ground floor (type 0 or others)
                baseTileId = getFloorTileId(semanticGrid, x, y);
            }
            
            if (baseTileId != -1) {
                tiles.push_back({x, y, baseTileId});
            }

            // Top boundary row: overlay a wall-front strip across y == 0.
            // This is a visual-only decoration; the movement block is handled
            // in Chamber::buildObstaclesFromGrid() via the row-0 baseObstacle.
            if (y == 0) {
                tiles.push_back({x, y, TileManager::getInstance().getTileId("wall-front")});
            }
        }
    }
    
    RenderableTileMap map;
    try {
        const sf::Texture& tex = AssetManager::getInstance().getTexture("dungeon-tiles");
        map.load(&tex, tiles, cellSize);
    } catch (...) {
        std::cerr << "Failed to get dungeon-tiles texture from AssetManager!" << std::endl;
    }
    map.setPosition(sf::Vector2f(offsetX, offsetY));
    
    return map;
}
