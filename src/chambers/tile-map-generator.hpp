#ifndef TILE_MAP_GENERATOR_HPP
#define TILE_MAP_GENERATOR_HPP

#include <SFML/Graphics.hpp>
#include <vector>

class RenderableTileMap : public sf::Drawable, public sf::Transformable {
public:
    RenderableTileMap();
    
    void load(const sf::Texture* texture, const std::vector<std::vector<int>>& tileIds, float cellSize);
    
private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    
    sf::VertexArray vertices;
    const sf::Texture* texture;
};

class TileMapGenerator {
public:
    // Takes a semantic grid (0=floor, 1=wall, 2=lake) and returns a ready-to-render TileMap
    static RenderableTileMap generate(const std::vector<std::vector<int>>& semanticGrid, float cellSize, float offsetX, float offsetY);
};

#endif // TILE_MAP_GENERATOR_HPP
