#include "tilemap-loader.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <queue>
#include <random>
#include <nlohmann/json.hpp>
#include "../global-settings/asset-manager.hpp"

using json = nlohmann::json;

TilemapLoader& TilemapLoader::getInstance() {
    static TilemapLoader instance;
    return instance;
}

TilemapLoader::TilemapLoader() {
    auto& tm = TileManager::getInstance();
    if (!tm.atlasLoaded) {
        tm.loadAtlasConfig("assets/animations/tile-map.json");
    }
}

TileType TilemapLoader::parseTileType(const std::string& typeStr) const {
    if (typeStr == "L") return TileType::Land;
    if (typeStr == "W") return TileType::Water;
    if (typeStr == "V") return TileType::VerticalBridge;
    if (typeStr == "H") return TileType::HorizontalBridge;
    return TileType::Void;
}



int TilemapLoader::tilePicker(TileType currentType, const std::vector<TileType>& neighbors) {
    // Stub implementation of tilePicker interface specified in Section 3
    if (currentType == TileType::Land) return 1;
    if (currentType == TileType::Water) return 2;
    if (currentType == TileType::VerticalBridge || currentType == TileType::HorizontalBridge) return 3;
    return 0;
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

    auto& tm = TileManager::getInstance();

    auto selectVariant = [](const std::vector<TileManager::TileAssetEntry>& matches) -> const TileManager::TileAssetEntry* {
        if (matches.empty()) return nullptr;
        std::map<int, std::vector<const TileManager::TileAssetEntry*>> byVariant;
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
            std::vector<const TileManager::TileAssetEntry*> nonZeroList;
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
        
        std::uniform_int_distribution<size_t> fbDist(0, matches.size() - 1);
        return &matches[fbDist(rng)];
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


                    // Match neighbor strings: 'L' for Land/water, 'V' for Void
                    std::string neighborStr = "";
                    for (int i = 0; i < 8; ++i) {
                        if (nTypes[i] == TileType::Land || nTypes[i] == TileType::VerticalBridge || nTypes[i] == TileType::HorizontalBridge || nTypes[i] == TileType::Water) {
                            neighborStr += 'L';
                        } else {
                            neighborStr += 'V';
                        }
                    }

                    // Check FILLED match ("LLLLLLLL")
                    std::vector<TileManager::TileAssetEntry> filledMatches;
                    for (const auto& item : tm.landAssets) {
                        if (item.position == "FILLED") {
                            bool match = true;
                            if (item.neighbors.length() != 8) match = false;
                            else {
                                for (size_t k = 0; k < 8; ++k) {
                                    if (item.neighbors[k] != '?' && item.neighbors[k] != neighborStr[k]) {
                                        match = false;
                                        break;
                                    }
                                }
                            }
                            if (match) filledMatches.push_back(item);
                        }
                    }

                    if (neighborStr == "LLLLLLLL" && !filledMatches.empty()) {
                        const TileManager::TileAssetEntry* chosen = selectVariant(filledMatches);
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
                            std::vector<TileManager::TileAssetEntry> qMatches;
                            for (const auto& item : tm.landAssets) {
                                if (item.position == quadConfigs[q].pos || item.position == "ANY") {
                                    bool match = true;
                                    if (item.neighbors.length() != quadConfigs[q].reqPattern.length()) match = false;
                                    else {
                                        for (size_t k = 0; k < item.neighbors.length(); ++k) {
                                            if (item.neighbors[k] != '?' && item.neighbors[k] != quadConfigs[q].reqPattern[k]) {
                                                match = false;
                                                break;
                                            }
                                        }
                                    }
                                    if (match) qMatches.push_back(item);
                                }
                            }
                            const TileManager::TileAssetEntry* chosen = selectVariant(qMatches);
                            if (!chosen) {
                                throw std::runtime_error("Land asset not found for position " + quadConfigs[q].pos + " with pattern " + quadConfigs[q].reqPattern);
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
                        std::vector<TileManager::TileAssetEntry> qMatches;
                        auto findMatches = [&](const std::string& pattern) {
                            qMatches.clear();
                            for (const auto& item : tm.waterAssets) {
                                if (item.position == quadConfigs[q].pos || item.position == "ANY") {
                                    bool match = true;
                                    if (item.neighbors.length() != pattern.length()) match = false;
                                    else {
                                        for (size_t k = 0; k < item.neighbors.length(); ++k) {
                                            if (item.neighbors[k] != '?' && item.neighbors[k] != pattern[k]) {
                                                match = false;
                                                break;
                                            }
                                        }
                                    }
                                    if (match) qMatches.push_back(item);
                                }
                            }
                        };
                        findMatches(quadConfigs[q].reqPattern);
                        const TileManager::TileAssetEntry* chosen = selectVariant(qMatches);
                        if (!chosen) {
                            throw std::runtime_error("Water asset not found for position " + quadConfigs[q].pos + " with pattern " + quadConfigs[q].reqPattern);
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
                            quad.texRect = {tm.softCliff.coord[0], tm.softCliff.coord[1], 16, 16};
                        } else if (step < delta) {
                            quad.texRect = {tm.semiHardCliff.coord[0], tm.semiHardCliff.coord[1], 16, 16};
                        }
                    } else if (typeBelow == TileType::Water) {
                        if (step == 1) {
                            quad.texRect = {tm.waterCliff.coord[0], tm.waterCliff.coord[1], 16, 16};
                        } else {
                            quad.texRect = {tm.hardCliff.coord[0], tm.hardCliff.coord[1], 16, 16};
                        }
                    } else { // Land
                        quad.texRect = {tm.hardCliff.coord[0], tm.hardCliff.coord[1], 16, 16};
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
                int shadowLen = endR - startR + 1;

                for (int dc = 0; dc < shadowLen; ++dc) {
                    int currR = startR + dc;
                    RenderTile shadowTile;
                    shadowTile.layer = levelGrid[currR][c] + 1;
                    shadowTile.gridRow = currR;
                    shadowTile.gridCol = c;

                    TileQuadInfo quad;
                    quad.renderOffset = {0.f, 0.f};
                    quad.renderSize = {16.f, 16.f};

                    if (dc == 0) {
                        quad.texRect = {tm.landShadowTop.coord[0], tm.landShadowTop.coord[1], 16, 16};
                    } else if (dc == shadowLen - 1) {
                        quad.texRect = {tm.landShadowBottom.coord[0], tm.landShadowBottom.coord[1], 16, 16};
                    } else {
                        quad.texRect = {tm.landShadowMiddle.coord[0], tm.landShadowMiddle.coord[1], 16, 16};
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
            TileType tType = parseTileType(typeGrid[r][c]);
            if (tType == TileType::VerticalBridge || tType == TileType::HorizontalBridge) {
                RenderTile bridgeTile;
                bridgeTile.layer = levelGrid[r][c] + 2;
                bridgeTile.gridRow = r;
                bridgeTile.gridCol = c;

                if (tType == TileType::VerticalBridge) {
                    TileQuadInfo quad;
                    quad.texRect = {tm.verticalBridge.coord[0], tm.verticalBridge.coord[1], tm.verticalBridge.size[0], tm.verticalBridge.size[1]};
                    quad.renderOffset = sf::Vector2f(0.f, 0.f);
                    quad.renderSize = sf::Vector2f(static_cast<float>(tm.verticalBridge.size[0]), static_cast<float>(tm.verticalBridge.size[1]));
                    bridgeTile.quads.push_back(quad);
                } else if (tType == TileType::HorizontalBridge) {
                    TileQuadInfo quad;
                    quad.texRect = {tm.horizontalBridge.coord[0], tm.horizontalBridge.coord[1], tm.horizontalBridge.size[0], tm.horizontalBridge.size[1]};
                    quad.renderOffset = sf::Vector2f(0.f, 0.f);
                    quad.renderSize = sf::Vector2f(static_cast<float>(tm.horizontalBridge.size[0]), static_cast<float>(tm.horizontalBridge.size[1]));
                    bridgeTile.quads.push_back(quad);
                }
                data.bridgeTiles.push_back(bridgeTile);
            }
        }
    }

    return data;
}



RenderableTileMap TilemapLoader::createRenderableMap(const std::vector<std::vector<std::string>>& typeGrid, const std::vector<std::vector<int>>& levelGrid, float cellSize, float offsetX, float offsetY) {
    TilemapRenderData renderData = synthesizeMap(typeGrid, levelGrid);
    RenderableTileMap map;
    try {
        const sf::Texture& tex = AssetManager::getInstance().getTexture("tile-map");
        map.loadFromRenderData(&tex, renderData, cellSize, offsetX, offsetY);
    } catch (...) {
        try {
            const sf::Texture& tex = AssetManager::getInstance().getTexture("dungeon-tiles");
            map.loadFromRenderData(&tex, renderData, cellSize, offsetX, offsetY);
        } catch (...) {
            std::cerr << "Failed to get tile-map texture from AssetManager!" << std::endl;
        }
    }
    map.setPosition(sf::Vector2f(offsetX, offsetY));
    return map;
}

