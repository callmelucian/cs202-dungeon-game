#ifndef TILE_MAP_GENERATOR_HPP
#define TILE_MAP_GENERATOR_HPP

#include <SFML/Graphics.hpp>
#include <vector>

struct TileData {
    int x, y, tileId;
};

class RenderableTileMap : public sf::Drawable, public sf::Transformable {
public:
    RenderableTileMap();
    
    void load(const sf::Texture* texture, const std::vector<TileData>& tiles, float cellSize);
    
private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    
    sf::VertexArray vertices;
    const sf::Texture* texture;
};

class TileMapGenerator {
public:
    // Takes a semantic grid and returns a ready-to-render TileMap.
    // Tile types: 0=floor, 1=wall, 2=lake, 4=elevated floor, 5=stairs, 6=cliff-face.
    // Ground tiles (type 0) are rendered via getFloorTileId() which selects
    // edge/corner variants based on map boundaries and adjacent elevated island tiles.
    static RenderableTileMap generate(const std::vector<std::vector<int>>& semanticGrid, float cellSize, float offsetX, float offsetY);
};

#endif // TILE_MAP_GENERATOR_HPP
