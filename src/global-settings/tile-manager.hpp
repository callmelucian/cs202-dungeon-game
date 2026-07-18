#ifndef TILE_MANAGER_HPP
#define TILE_MANAGER_HPP

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>

class TileManager {
public:
    static TileManager& getInstance();

    TileManager(const TileManager&) = delete;
    TileManager& operator=(const TileManager&) = delete;

    bool loadTileMap(const std::string& filepath);
    
    sf::IntRect getTileRect(int id) const;
    sf::IntRect getTileRect(const std::string& name) const;
    int getTileId(const std::string& name) const;
    
    sf::Vector2i getTileSize() const { return tileSize; }

private:
    TileManager() = default;
    ~TileManager() = default;

    struct TileData {
        std::string name;
        sf::IntRect rect;
    };

    std::unordered_map<int, TileData> tilesById;
    std::unordered_map<std::string, int> nameToId;
    sf::Vector2i tileSize;
};

#endif
