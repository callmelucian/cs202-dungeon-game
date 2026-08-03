#include "tile-manager.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

TileManager& TileManager::getInstance() {
    static TileManager instance;
    return instance;
}

bool TileManager::loadAtlasConfig(const std::string& configPath) {
    std::string path = configPath;
    if (!std::filesystem::exists(path) && std::filesystem::exists("../" + path)) {
        path = "../" + path;
    }
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "TileManager: Failed to open tile-map.json at " << path << std::endl;
        return false;
    }

    try {
        json j;
        file >> j;

        if (j.contains("tile-textures")) {
            const auto& tt = j["tile-textures"];
            if (tt.contains("LAND") && tt["LAND"].is_array()) {
                for (const auto& item : tt["LAND"]) {
                    TileAssetEntry entry;
                    entry.coord = item.value("coord", std::vector<int>{0, 0});
                    entry.position = item.value("position", "FILLED");
                    entry.neighbors = item.value("neighbors", "");
                    entry.textureVariant = item.value("texture", 0);
                    landAssets.push_back(entry);
                }
            }

            if (tt.contains("WATER") && tt["WATER"].is_array()) {
                for (const auto& item : tt["WATER"]) {
                    TileAssetEntry entry;
                    entry.coord = item.value("coord", std::vector<int>{0, 0});
                    entry.position = item.value("position", "ANY");
                    entry.neighbors = item.value("neighbors", "");
                    entry.textureVariant = item.value("texture", 0);
                    waterAssets.push_back(entry);
                }
            }

            auto loadOverlay = [](const json& src, OverlayAssetEntry& dst) {
                if (src.contains("coord")) dst.coord = src["coord"].get<std::vector<int>>();
                if (src.contains("size")) dst.size = src["size"].get<std::vector<int>>();
                else dst.size = {16, 16};
            };

            // Shadow overlays are handled pixel-by-pixel, no tile assets needed.
        }

        if (j.contains("cliffs")) {
            const auto& cliffs = j["cliffs"];
            auto loadCliff = [](const json& src, CliffAssetEntry& dst) {
                if (src.contains("coord")) dst.coord = src["coord"].get<std::vector<int>>();
                dst.position = src.value("position", "FILLED");
            };
            if (cliffs.contains("hard-cliff")) loadCliff(cliffs["hard-cliff"], hardCliff);
            if (cliffs.contains("semi-hard-cliff")) loadCliff(cliffs["semi-hard-cliff"], semiHardCliff);
            if (cliffs.contains("soft-cliff")) loadCliff(cliffs["soft-cliff"], softCliff);
            if (cliffs.contains("semi-soft-cliff")) loadCliff(cliffs["semi-soft-cliff"], semiSoftCliff);
            if (cliffs.contains("water-cliff")) loadCliff(cliffs["water-cliff"], waterCliff);
            if (cliffs.contains("stairs")) loadCliff(cliffs["stairs"], stairs);
            if (cliffs.contains("shadowed-stairs")) loadCliff(cliffs["shadowed-stairs"], shadowedStairs);
        }

        if (j.contains("overlays")) {
            const auto& overlays = j["overlays"];
            if (overlays.contains("vertical-bridge")) {
                verticalBridge.coord = overlays["vertical-bridge"].value("coord", std::vector<int>{165, 130});
                verticalBridge.size = overlays["vertical-bridge"].value("size", std::vector<int>{16, 20});
            }
            if (overlays.contains("horizontal-bridge")) {
                horizontalBridge.coord = overlays["horizontal-bridge"].value("coord", std::vector<int>{140, 132});
                horizontalBridge.size = overlays["horizontal-bridge"].value("size", std::vector<int>{20, 16});
            }
        }

        atlasLoaded = true;
        return true;
    } catch (const json::exception& e) {
        std::cerr << "TileManager: Error parsing JSON: " << e.what() << std::endl;
        return false;
    }
}
