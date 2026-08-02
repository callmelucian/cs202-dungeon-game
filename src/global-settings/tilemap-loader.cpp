#include "tilemap-loader.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <queue>
#include <random>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

TilemapLoader& TilemapLoader::getInstance() {
    static TilemapLoader instance;
    return instance;
}

TilemapLoader::TilemapLoader() {
    loadAtlasConfig("assets/animations/tile-map.json");
}

TileType TilemapLoader::parseTileType(const std::string& typeStr) const {
    if (typeStr == "L") return TileType::Land;
    if (typeStr == "W") return TileType::Water;
    if (typeStr == "V") return TileType::VerticalBridge;
    if (typeStr == "H") return TileType::HorizontalBridge;
    return TileType::Void;
}

void TilemapLoader::loadAtlasConfig(const std::string& configPath) {
    std::string path = configPath;
    if (!std::filesystem::exists(path) && std::filesystem::exists("../" + path)) {
        path = "../" + path;
    }
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "TilemapLoader: Failed to open tile-map.json at " << path << std::endl;
        return;
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

            if (tt.contains("LAND-SHADOWED-TOP")) loadOverlay(tt["LAND-SHADOWED-TOP"], landShadowTop);
            if (tt.contains("LAND-SHADOWED-MIDDLE")) loadOverlay(tt["LAND-SHADOWED-MIDDLE"], landShadowMiddle);
            if (tt.contains("LAND-SHADOWED-BOTTOM")) loadOverlay(tt["LAND-SHADOWED-BOTTOM"], landShadowBottom);
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
            if (cliffs.contains("water-cliff")) loadCliff(cliffs["water-cliff"], waterCliff);
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
    } catch (const json::exception& e) {
        std::cerr << "TilemapLoader: Error parsing JSON: " << e.what() << std::endl;
    }
}

int TilemapLoader::tilePicker(TileType currentType, const std::vector<TileType>& neighbors) {
    // Stub implementation of tilePicker interface specified in Section 3
    if (currentType == TileType::Land) return 1;
    if (currentType == TileType::Water) return 2;
    if (currentType == TileType::VerticalBridge || currentType == TileType::HorizontalBridge) return 3;
    return 0;
}

std::vector<std::vector<int>> TilemapLoader::loadMap(
    const std::vector<std::vector<std::string>>& typeGrid,
    const std::vector<std::vector<int>>& levelGrid
) {
    TilemapRenderData data = synthesizeMap(typeGrid, levelGrid);
    return data.legacyGrid;
}

TilemapRenderData TilemapLoader::synthesizeMap(
    const std::vector<std::vector<std::string>>& typeGrid,
    const std::vector<std::vector<int>>& levelGrid
) {
    TilemapRenderData data;
    if (typeGrid.empty() || typeGrid[0].empty()) return data;

    int rows = static_cast<int>(typeGrid.size());
    int cols = static_cast<int>(typeGrid[0].size());
    data.width = cols;
    data.height = rows;

    data.legacyGrid = std::vector<std::vector<int>>(rows, std::vector<int>(cols, 0));
    data.walkableGrid = std::vector<std::vector<bool>>(rows, std::vector<bool>(cols, false));

    // Determine elevation level range
    int minLevel = 1;
    int maxLevel = 1;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (levelGrid[r][c] > maxLevel) maxLevel = levelGrid[r][c];
        }
    }

    static std::mt19937 rng(42);

    auto selectVariant = [&](const std::vector<TileAssetEntry>& matches) -> const TileAssetEntry* {
        if (matches.empty()) return nullptr;
        std::map<int, std::vector<const TileAssetEntry*>> byVariant;
        for (const auto& item : matches) {
            byVariant[item.textureVariant].push_back(&item);
        }
        int numNonZero = 0;
        for (const auto& kv : byVariant) {
            if (kv.first > 0) numNonZero++;
        }

        double probZero = (numNonZero > 0) ? 0.9 : 1.0;
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        double roll = dist(rng);

        if (roll < probZero && byVariant.count(0) && !byVariant[0].empty()) {
            std::uniform_int_distribution<size_t> idxDist(0, byVariant[0].size() - 1);
            return byVariant[0][idxDist(rng)];
        } else if (numNonZero > 0) {
            std::vector<const TileAssetEntry*> nonZeroList;
            for (const auto& kv : byVariant) {
                if (kv.first > 0) {
                    nonZeroList.insert(nonZeroList.end(), kv.second.begin(), kv.second.end());
                }
            }
            if (!nonZeroList.empty()) {
                std::uniform_int_distribution<size_t> idxDist(0, nonZeroList.size() - 1);
                return nonZeroList[idxDist(rng)];
            }
        }
        std::uniform_int_distribution<size_t> idxDist(0, matches.size() - 1);
        return &matches[idxDist(rng)];
    };

    // Helper: Find 4-connected components for level >= L
    for (int L = minLevel; L <= maxLevel; ++L) {
        std::vector<std::vector<int>> compId(rows, std::vector<int>(cols, -1));
        int currentComp = 0;

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                if (levelGrid[r][c] >= L && compId[r][c] == -1) {
                    std::queue<std::pair<int, int>> q;
                    q.push({r, c});
                    compId[r][c] = currentComp;

                    while (!q.empty()) {
                        auto [currR, currC] = q.front();
                        q.pop();

                        const int dr[] = {-1, 1, 0, 0};
                        const int dc[] = {0, 0, -1, 1};
                        for (int i = 0; i < 4; ++i) {
                            int nr = currR + dr[i];
                            int nc = currC + dc[i];
                            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                                if (levelGrid[nr][nc] >= L && compId[nr][nc] == -1) {
                                    compId[nr][nc] = currentComp;
                                    q.push({nr, nc});
                                }
                            }
                        }
                    }
                    currentComp++;
                }
            }
        }

        // Render pass for level L from top row to bottom row
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                if (levelGrid[r][c] < L) continue;

                TileType cellType = parseTileType(typeGrid[r][c]);
                if (cellType == TileType::Void) continue;

                // Rule: Label higher-level cells as Land for current pass
                if (levelGrid[r][c] > L) {
                    cellType = TileType::Land;
                }

                int myComp = compId[r][c];

                // Gather 8 neighbors in row-major order: 1..8
                // Index mapping: 0:TL, 1:T, 2:TR, 3:L, 4:R, 5:BL, 6:B, 7:BR
                const int nRow[] = {-1, -1, -1,  0, 0,  1, 1, 1};
                const int nCol[] = {-1,  0,  1, -1, 1, -1, 0, 1};
                std::vector<TileType> nTypes(8, TileType::Void);

                for (int i = 0; i < 8; ++i) {
                    int nr = r + nRow[i];
                    int nc = c + nCol[i];
                    if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                        if (levelGrid[nr][nc] >= L && compId[nr][nc] == myComp) {
                            TileType nType = parseTileType(typeGrid[nr][nc]);
                            if (levelGrid[nr][nc] > L) {
                                nType = TileType::Land;
                            }
                            nTypes[i] = nType;
                        }
                    }
                }

                // Autotilingquad assembly
                RenderTile baseTile;
                baseTile.layer = L;
                baseTile.gridRow = r;
                baseTile.gridCol = c;

                if (cellType == TileType::Land || cellType == TileType::VerticalBridge || cellType == TileType::HorizontalBridge) {
                    data.walkableGrid[r][c] = true;
                    data.legacyGrid[r][c] = (cellType == TileType::Land) ? 0 : 3;

                    // Match neighbor strings: 'L' for Land/same comp, 'V' for Void/different
                    std::string neighborStr = "";
                    for (int i = 0; i < 8; ++i) {
                        if (nTypes[i] == TileType::Land || nTypes[i] == TileType::VerticalBridge || nTypes[i] == TileType::HorizontalBridge) {
                            neighborStr += 'L';
                        } else {
                            neighborStr += 'V';
                        }
                    }

                    // Check FILLED match ("LLLLLLLL")
                    std::vector<TileAssetEntry> filledMatches;
                    for (const auto& item : landAssets) {
                        if (item.position == "FILLED" && item.neighbors == "LLLLLLLL") {
                            filledMatches.push_back(item);
                        }
                    }

                    if (neighborStr == "LLLLLLLL" && !filledMatches.empty()) {
                        const TileAssetEntry* chosen = selectVariant(filledMatches);
                        if (chosen) {
                            TileQuadInfo quad;
                            quad.texRect = {chosen->coord[0], chosen->coord[1], 16, 16};
                            quad.renderOffset = {0.f, 0.f};
                            quad.renderSize = {16.f, 16.f};
                            baseTile.quads.push_back(quad);
                        }
                    } else {
                        // Assemble 4 quadrants ($8 \times 8$ sub-tiles)
                        // TL: (0, 1, 3) -> TL, Top, Left
                        // TR: (1, 2, 4) -> Top, TR, Right
                        // BL: (3, 5, 6) -> Left, BL, Bottom
                        // BR: (4, 6, 7) -> Right, Bottom, BR
                        struct QuadConfig {
                            std::string pos;
                            std::string reqPattern;
                            sf::Vector2f offset;
                        };

                        std::string tlPattern = std::string(1, neighborStr[0]) + neighborStr[1] + neighborStr[3];
                        std::string trPattern = std::string(1, neighborStr[1]) + neighborStr[2] + neighborStr[4];
                        std::string blPattern = std::string(1, neighborStr[3]) + neighborStr[5] + neighborStr[6];
                        std::string brPattern = std::string(1, neighborStr[4]) + neighborStr[6] + neighborStr[7];

                        QuadConfig quadConfigs[4] = {
                            {"TL", tlPattern, {0.f, 0.f}},
                            {"TR", trPattern, {8.f, 0.f}},
                            {"BL", blPattern, {0.f, 8.f}},
                            {"BR", brPattern, {8.f, 8.f}}
                        };

                        for (int q = 0; q < 4; ++q) {
                            std::vector<TileAssetEntry> qMatches;
                            for (const auto& item : landAssets) {
                                if ((item.position == quadConfigs[q].pos || item.position == "ANY") &&
                                    item.neighbors == quadConfigs[q].reqPattern) {
                                    qMatches.push_back(item);
                                }
                            }
                            const TileAssetEntry* chosen = selectVariant(qMatches);
                            if (!chosen) {
                                // Fallback to ANY LLL
                                for (const auto& item : landAssets) {
                                    if (item.position == "ANY" && item.neighbors == "LLL") {
                                        qMatches.push_back(item);
                                    }
                                }
                                chosen = selectVariant(qMatches);
                            }
                            if (chosen) {
                                TileQuadInfo quad;
                                quad.texRect = {chosen->coord[0], chosen->coord[1], 8, 8};
                                quad.renderOffset = quadConfigs[q].offset;
                                quad.renderSize = {8.f, 8.f};
                                baseTile.quads.push_back(quad);
                            }
                        }
                    }
                    data.baseTiles.push_back(baseTile);
                } else if (cellType == TileType::Water) {
                    data.legacyGrid[r][c] = 2; // Water lake
                    std::string neighborStr = "";
                    for (int i = 0; i < 8; ++i) {
                        if (nTypes[i] == TileType::Water) neighborStr += 'W';
                        else neighborStr += 'L';
                    }

                    std::string tlPattern = std::string(1, neighborStr[0]) + neighborStr[1] + neighborStr[3];
                    std::string trPattern = std::string(1, neighborStr[1]) + neighborStr[2] + neighborStr[4];
                    std::string blPattern = std::string(1, neighborStr[3]) + neighborStr[5] + neighborStr[6];
                    std::string brPattern = std::string(1, neighborStr[4]) + neighborStr[6] + neighborStr[7];

                    struct QuadConfig {
                        std::string pos;
                        std::string reqPattern;
                        sf::Vector2f offset;
                    } quadConfigs[4] = {
                        {"TL", tlPattern, {0.f, 0.f}},
                        {"TR", trPattern, {8.f, 0.f}},
                        {"BL", blPattern, {0.f, 8.f}},
                        {"BR", brPattern, {8.f, 8.f}}
                    };

                    for (int q = 0; q < 4; ++q) {
                        std::vector<TileAssetEntry> qMatches;
                        for (const auto& item : waterAssets) {
                            if ((item.position == quadConfigs[q].pos || item.position == "ANY") &&
                                item.neighbors == quadConfigs[q].reqPattern) {
                                qMatches.push_back(item);
                            }
                        }
                        const TileAssetEntry* chosen = selectVariant(qMatches);
                        if (!chosen) {
                            for (const auto& item : waterAssets) {
                                if (item.position == "ANY" && item.neighbors == "WWW") {
                                    qMatches.push_back(item);
                                }
                            }
                            chosen = selectVariant(qMatches);
                        }
                        if (chosen) {
                            TileQuadInfo quad;
                            quad.texRect = {chosen->coord[0], chosen->coord[1], 8, 8};
                            quad.renderOffset = quadConfigs[q].offset;
                            quad.renderSize = {8.f, 8.f};
                            baseTile.quads.push_back(quad);
                        }
                    }
                    data.baseTiles.push_back(baseTile);
                }
            }
        }
    }

    // Section 5.2: Cliff Generation Logic
    for (int r = 0; r < rows - 1; ++r) {
        for (int c = 0; c < cols; ++c) {
            int levCurrent = levelGrid[r][c];
            int levBelow = levelGrid[r+1][c];
            if (levCurrent > levBelow) {
                int delta = levCurrent - levBelow;
                TileType typeBelow = parseTileType(typeGrid[r+1][c]);

                for (int step = 1; step <= delta; ++step) {
                    RenderTile cliffTile;
                    cliffTile.layer = levCurrent;
                    cliffTile.gridRow = r + step;
                    cliffTile.gridCol = c;

                    TileQuadInfo quad;
                    quad.renderOffset = {0.f, 0.f};
                    quad.renderSize = {16.f, 16.f};

                    if (typeBelow == TileType::Void || levBelow == 0) {
                        if (step == delta) {
                            quad.texRect = {softCliff.coord[0], softCliff.coord[1], 16, 16};
                        } else {
                            quad.texRect = {semiHardCliff.coord[0], semiHardCliff.coord[1], 16, 16};
                        }
                    } else if (typeBelow == TileType::Water) {
                        if (step == delta) {
                            quad.texRect = {waterCliff.coord[0], waterCliff.coord[1], 16, 16};
                        } else {
                            quad.texRect = {hardCliff.coord[0], hardCliff.coord[1], 16, 16};
                        }
                    } else { // Land
                        quad.texRect = {hardCliff.coord[0], hardCliff.coord[1], 16, 16};
                    }

                    cliffTile.quads.push_back(quad);
                    data.cliffTiles.push_back(cliffTile);
                }
            }
        }
    }

    // Section 5.3: Shadowing Logic
    for (int c = 1; c < cols; ++c) {
        int r = 0;
        while (r < rows) {
            if (parseTileType(typeGrid[r][c]) == TileType::Land &&
                parseTileType(typeGrid[r][c-1]) == TileType::Land &&
                levelGrid[r][c-1] > levelGrid[r][c]) {

                int startR = r;
                while (r < rows &&
                       parseTileType(typeGrid[r][c]) == TileType::Land &&
                       parseTileType(typeGrid[r][c-1]) == TileType::Land &&
                       levelGrid[r][c-1] > levelGrid[r][c]) {
                    r++;
                }
                int endR = r - 1;
                int len = endR - startR + 1;

                for (int i = 0; i < len; ++i) {
                    int currR = startR + i;
                    RenderTile shadowTile;
                    shadowTile.layer = levelGrid[currR][c] + 1;
                    shadowTile.gridRow = currR;
                    shadowTile.gridCol = c;

                    TileQuadInfo quad;
                    quad.renderOffset = {0.f, 0.f};
                    quad.renderSize = {16.f, 16.f};

                    if (len == 1 || i == 0) {
                        quad.texRect = {landShadowTop.coord[0], landShadowTop.coord[1], 16, 16};
                    } else if (i == len - 1) {
                        quad.texRect = {landShadowBottom.coord[0], landShadowBottom.coord[1], 16, 16};
                    } else {
                        quad.texRect = {landShadowMiddle.coord[0], landShadowMiddle.coord[1], 16, 16};
                    }
                    shadowTile.quads.push_back(quad);
                    data.shadowTiles.push_back(shadowTile);
                }
            } else {
                r++;
            }
        }
    }

    // Section 5.4: Bridge Overlays
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            TileType t = parseTileType(typeGrid[r][c]);
            if (t == TileType::VerticalBridge || t == TileType::HorizontalBridge) {
                RenderTile bridgeTile;
                bridgeTile.layer = levelGrid[r][c] + 2;
                bridgeTile.gridRow = r;
                bridgeTile.gridCol = c;

                TileQuadInfo quad;
                if (t == TileType::VerticalBridge) {
                    quad.texRect = {verticalBridge.coord[0], verticalBridge.coord[1], verticalBridge.size[0], verticalBridge.size[1]};
                    quad.renderOffset = {0.f, -2.f};
                    quad.renderSize = {static_cast<float>(verticalBridge.size[0]), static_cast<float>(verticalBridge.size[1])};
                } else {
                    quad.texRect = {horizontalBridge.coord[0], horizontalBridge.coord[1], horizontalBridge.size[0], horizontalBridge.size[1]};
                    quad.renderOffset = {-2.f, 0.f};
                    quad.renderSize = {static_cast<float>(horizontalBridge.size[0]), static_cast<float>(horizontalBridge.size[1])};
                }
                bridgeTile.quads.push_back(quad);
                data.bridgeTiles.push_back(bridgeTile);
            }
        }
    }

    return data;
}
