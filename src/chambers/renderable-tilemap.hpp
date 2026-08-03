#ifndef RENDERABLE_TILEMAP_HPP
#define RENDERABLE_TILEMAP_HPP

#include <vector>
#include <string>
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
    std::vector<std::vector<bool>> walkableGrid;
};

class RenderableTileMap : public sf::Drawable, public sf::Transformable {
public:
    RenderableTileMap();
    
    void loadFromRenderData(const sf::Texture* texture, const TilemapRenderData& renderData, float cellSize, float offsetX, float offsetY);
    
private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    
    sf::VertexArray vertices;
    const sf::Texture* texture;
};

#endif // RENDERABLE_TILEMAP_HPP
