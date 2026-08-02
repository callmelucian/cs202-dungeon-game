#ifndef TILE_MAP_GENERATOR_HPP
#define TILE_MAP_GENERATOR_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include "../global-settings/tilemap-loader.hpp"

struct TileData {
    int x, y, tileId;
};

class RenderableTileMap : public sf::Drawable, public sf::Transformable {
public:
    RenderableTileMap();
    
    void load(const sf::Texture* texture, const std::vector<TileData>& tiles, float cellSize);
    void loadFromRenderData(const sf::Texture* texture, const TilemapRenderData& renderData, float cellSize, float offsetX, float offsetY);
    
private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    
    sf::VertexArray vertices;
    const sf::Texture* texture;
};

class TileMapGenerator {
public:
    // Legacy generator wrapper
    static RenderableTileMap generate(const std::vector<std::vector<int>>& semanticGrid, float cellSize, float offsetX, float offsetY);
    
    // 2.5D generator using TilemapLoader
    static RenderableTileMap generate2D5(const std::vector<std::vector<std::string>>& typeGrid, const std::vector<std::vector<int>>& levelGrid, float cellSize, float offsetX, float offsetY);
};

#endif // TILE_MAP_GENERATOR_HPP
