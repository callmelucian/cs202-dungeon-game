#include "tile-manager.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

TileManager& TileManager::getInstance() {
    static TileManager instance;
    return instance;
}

bool TileManager::loadTileMap(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open tile map: " << filepath << std::endl;
        return false;
    }

    nlohmann::json j;
    try {
        file >> j;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parse error in " << filepath << ": " << e.what() << std::endl;
        return false;
    }

    tileSize.x = j.value("tile-width", 16);
    tileSize.y = j.value("tile-height", 16);

    if (j.contains("tiles") && j["tiles"].is_array()) {
        for (const auto& tileJson : j["tiles"]) {
            int id = tileJson.value("id", -1);
            if (id == -1) continue;

            std::string name = tileJson.value("name", "");
            int x = tileJson.value("x", 0);
            int y = tileJson.value("y", 0);

            TileData data;
            data.name = name;
            data.rect = sf::IntRect({x, y}, tileSize);

            tilesById[id] = data;
            nameToId[name] = id;
        }
    }

    return true;
}

sf::IntRect TileManager::getTileRect(int id) const {
    auto it = tilesById.find(id);
    if (it != tilesById.end()) {
        return it->second.rect;
    }
    return sf::IntRect({0, 0}, tileSize); // Fallback
}

sf::IntRect TileManager::getTileRect(const std::string& name) const {
    auto idIt = nameToId.find(name);
    if (idIt != nameToId.end()) {
        return getTileRect(idIt->second);
    }
    return sf::IntRect({0, 0}, tileSize);
}

int TileManager::getTileId(const std::string& name) const {
    auto idIt = nameToId.find(name);
    if (idIt != nameToId.end()) {
        return idIt->second;
    }
    return -1;
}
