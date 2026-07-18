#include "tile-map-generator.hpp"
#include "../global-settings/tile-manager.hpp"
#include "../global-settings/asset-manager.hpp"
#include <iostream>
#include <cstdlib>

RenderableTileMap::RenderableTileMap() 
    : vertices(sf::PrimitiveType::Triangles), texture(nullptr) {}

void RenderableTileMap::load(const sf::Texture* tex, const std::vector<std::vector<int>>& tileIds, float cellSize) {
    texture = tex;
    
    if (tileIds.empty() || tileIds[0].empty()) return;
    
    size_t width = tileIds[0].size();
    size_t height = tileIds.size();
    
    vertices.resize(width * height * 6);
    
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            int tileId = tileIds[y][x];
            sf::IntRect texRect = TileManager::getInstance().getTileRect(tileId);
            
            // Generate 6 vertices for a quad (2 triangles)
            sf::Vertex* quad = &vertices[(x + y * width) * 6];
            
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
}

void RenderableTileMap::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    states.texture = texture;
    target.draw(vertices, states);
}

// 9-slice mapping logic
static int getTileIdFor9Slice(const std::vector<std::vector<int>>& grid, int x, int y, int type, const std::string& prefix, bool invert = false) {
    bool wallUp = (y == 0 || grid[y - 1][x] == type);
    bool wallDown = (y == grid.size() - 1 || grid[y + 1][x] == type);
    bool wallLeft = (x == 0 || grid[y][x - 1] == type);
    bool wallRight = (x == grid[y].size() - 1 || grid[y][x + 1] == type);

    bool u = invert ? wallDown : wallUp;
    bool d = invert ? wallUp : wallDown;
    bool l = invert ? wallRight : wallLeft;
    bool r = invert ? wallLeft : wallRight;

    std::string suffix;
    
    // Explicit corner handling for outer bounds (since they are 1-thick walls surrounded by out-of-bounds walls)
    if (invert && x == 0 && y == 0) suffix = "upper-left";
    else if (invert && x == grid[y].size() - 1 && y == 0) suffix = "upper-right";
    else if (invert && x == 0 && y == grid.size() - 1) suffix = "lower-left";
    else if (invert && x == grid[y].size() - 1 && y == grid.size() - 1) suffix = "lower-right";
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

RenderableTileMap TileMapGenerator::generate(const std::vector<std::vector<int>>& semanticGrid, float cellSize, float offsetX, float offsetY) {
    if (semanticGrid.empty() || semanticGrid[0].empty()) return RenderableTileMap();
    
    size_t width = semanticGrid[0].size();
    size_t height = semanticGrid.size();
    
    std::vector<std::vector<int>> tileIds(height, std::vector<int>(width, 0));
    
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            int type = semanticGrid[y][x];
            if (type == 1) { // Wall
                tileIds[y][x] = getTileIdFor9Slice(semanticGrid, x, y, 1, "tile", true);
            } else if (type == 2) { // Lake
                tileIds[y][x] = getTileIdFor9Slice(semanticGrid, x, y, 2, "lake");
            } else { // Floor (type 0 or others)
                if (std::rand() % 100 < 5) {
                    tileIds[y][x] = TileManager::getInstance().getTileId("tile-center-with-stone");
                } else {
                    tileIds[y][x] = TileManager::getInstance().getTileId("tile-center");
                }
            }
        }
    }
    
    RenderableTileMap map;
    try {
        const sf::Texture& tex = AssetManager::getInstance().getTexture("dungeon-tiles");
        map.load(&tex, tileIds, cellSize);
    } catch (...) {
        std::cerr << "Failed to get dungeon-tiles texture from AssetManager!" << std::endl;
    }
    map.setPosition(sf::Vector2f(offsetX, offsetY));
    
    return map;
}
