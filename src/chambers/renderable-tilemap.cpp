#include "renderable-tilemap.hpp"
#include <algorithm>

RenderableTileMap::RenderableTileMap() 
    : vertices(sf::PrimitiveType::Triangles), untexturedVertices(sf::PrimitiveType::Triangles), texture(nullptr) {}

void RenderableTileMap::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    
    sf::RenderStates texStates = states;
    texStates.texture = texture;
    target.draw(vertices, texStates);
    
    sf::RenderStates untexStates = states;
    untexStates.texture = nullptr;
    target.draw(untexturedVertices, untexStates);
}

void RenderableTileMap::loadFromRenderData(const sf::Texture* tex, const TilemapRenderData& renderData, float cellSize, float offsetX, float offsetY) {
    texture = tex;
    
    std::vector<const RenderTile*> allTileLists;
    for (const auto& t : renderData.baseTiles) allTileLists.push_back(&t);
    for (const auto& t : renderData.cliffTiles) allTileLists.push_back(&t);
    for (const auto& t : renderData.shadowTiles) allTileLists.push_back(&t);
    for (const auto& t : renderData.bridgeTiles) allTileLists.push_back(&t);

    std::stable_sort(allTileLists.begin(), allTileLists.end(), [](const RenderTile* a, const RenderTile* b) {
        if (a->layer != b->layer) return a->layer < b->layer;
        if (a->gridRow != b->gridRow) return a->gridRow < b->gridRow;
        return a->gridCol < b->gridCol;
    });

    size_t totalTexturedQuads = 0;
    size_t totalUntexturedQuads = 0;
    for (const auto* rt : allTileLists) {
        for (const auto& q : rt->quads) {
            if (q.isTextured) totalTexturedQuads++;
            else totalUntexturedQuads++;
        }
    }

    vertices.resize(totalTexturedQuads * 6);
    untexturedVertices.resize(totalUntexturedQuads * 6);
    size_t texQuadIndex = 0;
    size_t untexQuadIndex = 0;

    float scale = cellSize / 16.0f;

    for (const auto* rt : allTileLists) {
        for (const auto& q : rt->quads) {
            float x = rt->gridCol * cellSize + q.renderOffset.x * scale;
            float y = rt->gridRow * cellSize + q.renderOffset.y * scale;
            float w = q.renderSize.x * scale;
            float h = q.renderSize.y * scale;

            sf::Vertex* v;
            if (q.isTextured) {
                v = &vertices[texQuadIndex * 6];
                texQuadIndex++;
            } else {
                v = &untexturedVertices[untexQuadIndex * 6];
                untexQuadIndex++;
            }

            v[0].position = sf::Vector2f(x, y);
            v[1].position = sf::Vector2f(x + w, y);
            v[2].position = sf::Vector2f(x, y + h);

            v[3].position = sf::Vector2f(x, y + h);
            v[4].position = sf::Vector2f(x + w, y);
            v[5].position = sf::Vector2f(x + w, y + h);

            for (int i = 0; i < 6; ++i) {
                v[i].color = q.color;
            }

            if (q.isTextured) {
                float tu = static_cast<float>(q.texRect.x);
                float tv = static_cast<float>(q.texRect.y);
                float tw = static_cast<float>(q.texRect.width);
                float th = static_cast<float>(q.texRect.height);

                v[0].texCoords = sf::Vector2f(tu, tv);
                v[1].texCoords = sf::Vector2f(tu + tw, tv);
                v[2].texCoords = sf::Vector2f(tu, tv + th);

                v[3].texCoords = sf::Vector2f(tu, tv + th);
                v[4].texCoords = sf::Vector2f(tu + tw, tv);
                v[5].texCoords = sf::Vector2f(tu + tw, tv + th);
            }
        }
    }
}
