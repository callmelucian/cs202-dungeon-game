#include "tilemap-loader.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <queue>
#include <random>
#include <stdexcept>
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
    if (typeStr == "S") return TileType::Stairs;
    if (typeStr == "0") return TileType::Void;
    if (typeStr == "E") return TileType::Land; // Echo visually sits on Land
    if (typeStr == "X") return TileType::Land; // Exit visually sits on Land
    throw std::invalid_argument("Invalid tile type string: " + typeStr);
}

int TilemapLoader::tilePicker(TileType currentType, const std::vector<TileType>& neighbors) {
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

    data.walkableGrid = std::vector<std::vector<uint8_t>>(rows, std::vector<uint8_t>(cols, 0));

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

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                if (levelGrid[r][c] < L) continue;

                TileType cellType = parseTileType(typeGrid[r][c]);
                if (cellType == TileType::Void) continue;

                if (levelGrid[r][c] > L) {
                    cellType = TileType::Land;
                }

                int myComp = compId[r][c];

                const int nRow[] = {-1, -1, -1,  0, 0,  1, 1, 1};
                const int nCol[] = {-1,  0,  1, -1, 1, -1, 0, 1};
                std::vector<TileType> nTypes(8, TileType::Void);
                std::vector<bool> nElevated(8, false);

                for (int i = 0; i < 8; ++i) {
                    int nr = r + nRow[i];
                    int nc = c + nCol[i];
                    if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                        if (levelGrid[nr][nc] >= L && compId[nr][nc] == myComp) {
                            TileType nType = parseTileType(typeGrid[nr][nc]);
                            if (levelGrid[nr][nc] > L) {
                                nType = TileType::Land;
                                nElevated[i] = true;
                            }
                            nTypes[i] = nType;
                        }
                    }
                }

                RenderTile baseTile;
                baseTile.layer = L;
                baseTile.gridRow = r;
                baseTile.gridCol = c;

                if (cellType == TileType::Land || cellType == TileType::Stairs || cellType == TileType::VerticalBridge || cellType == TileType::HorizontalBridge) {
                    
                    std::string neighborTypes = "";
                    for (int i = 0; i < 8; ++i) {
                        if (nTypes[i] == TileType::Land || nTypes[i] == TileType::Stairs || nTypes[i] == TileType::VerticalBridge || nTypes[i] == TileType::HorizontalBridge || nTypes[i] == TileType::Water) {
                            neighborTypes += 'L';
                        } else {
                            neighborTypes += 'V';
                        }
                    }

                    auto getMatches = [&](const std::string& pos, const std::vector<int>& indices, bool ignoreElevation) {
                        std::vector<TileManager::TileAssetEntry> matches;
                        for (const auto& item : tm.landAssets) {
                            if (item.position == pos || item.position == "ANY") {
                                bool match = true;
                                if (item.neighbors.length() != indices.size()) match = false;
                                else {
                                    for (size_t k = 0; k < indices.size(); ++k) {
                                        int idx = indices[k];
                                        char ap = item.neighbors[k];
                                        bool isElev = ignoreElevation ? false : nElevated[idx];
                                        char nt = neighborTypes[idx];
                                        
                                        if (ap == '?') continue;
                                        if (ap == 'E' && !isElev) { match = false; break; }
                                        if (ap == '-' && isElev) { match = false; break; }
                                        if (ap == 'L' && nt != 'L') { match = false; break; }
                                        if (ap == 'V' && nt != 'V') { match = false; break; }
                                    }
                                }
                                if (match) matches.push_back(item);
                            }
                        }
                        return matches;
                    };

                    std::vector<int> filledIdx = {0, 1, 2, 3, 4, 5, 6, 7};
                    auto filledMatches = getMatches("FILLED", filledIdx, false);
                    if (filledMatches.empty()) filledMatches = getMatches("FILLED", filledIdx, true);

                    if (neighborTypes == "LLLLLLLL" && !filledMatches.empty()) {
                        const TileManager::TileAssetEntry* chosen = selectVariant(filledMatches);
                        if (chosen) {
                            TileQuadInfo quad;
                            quad.texRect = {chosen->coord[0], chosen->coord[1], 16, 16};
                            quad.renderOffset = {0.f, 0.f};
                            quad.renderSize = {16.f, 16.f};
                            baseTile.quads.push_back(quad);
                        }
                    } else {
                        struct QuadConfig {
                            std::string pos;
                            std::vector<int> indices;
                            sf::Vector2f offset;
                        };
                        QuadConfig quadConfigs[4] = {
                            {"TL", {0, 1, 3}, {0.f, 0.f}},
                            {"TR", {1, 2, 4}, {8.f, 0.f}},
                            {"BL", {3, 5, 6}, {0.f, 8.f}},
                            {"BR", {4, 6, 7}, {8.f, 8.f}}
                        };

                        for (int q = 0; q < 4; ++q) {
                            auto qMatches = getMatches(quadConfigs[q].pos, quadConfigs[q].indices, false);
                            if (qMatches.empty()) {
                                qMatches = getMatches(quadConfigs[q].pos, quadConfigs[q].indices, true);
                            }
                            const TileManager::TileAssetEntry* chosen = selectVariant(qMatches);
                            if (!chosen) {
                                throw std::runtime_error("Land asset not found for quadrant");
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
                        for (const auto& item : tm.waterAssets) {
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
                            throw std::runtime_error("Water asset not found");
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

    for (int r = 0; r < rows - 1; ++r) {
        for (int c = 0; c < cols; ++c) {
            int levCurrent = levelGrid[r][c];
            int levBelow = levelGrid[r+1][c];
            if (levCurrent > levBelow) {
                int delta = levCurrent - levBelow;
                TileType typeBelow = parseTileType(typeGrid[r+1][c]);

                bool hasLeftHardCliff = false;
                if (c > 0) {
                     if (levelGrid[r][c-1] > levelGrid[r+1][c-1]) {
                         TileType tbLeft = parseTileType(typeGrid[r+1][c-1]);
                         if (tbLeft == TileType::Land || tbLeft == TileType::Water) {
                             hasLeftHardCliff = true;
                         }
                     }
                }

                for (int step = 1; step <= delta; ++step) {
                    RenderTile cliffTile;
                    cliffTile.layer = levCurrent;
                    cliffTile.gridRow = r + step;
                    cliffTile.gridCol = c;

                    TileQuadInfo quad;
                    quad.renderOffset = {0.f, 0.f};
                    quad.renderSize = {16.f, 16.f};
                    quad.isTextured = true;

                    if (typeBelow == TileType::Void || levBelow == 0) {
                        if (delta == 1) {
                            quad.texRect = {tm.semiSoftCliff.coord[0], tm.semiSoftCliff.coord[1], 16, 16};
                        } else if (step == delta) {
                            quad.texRect = {tm.softCliff.coord[0], tm.softCliff.coord[1], 16, 16};
                        } else {
                            quad.texRect = {tm.semiHardCliff.coord[0], tm.semiHardCliff.coord[1], 16, 16};
                        }
                    } else if (typeBelow == TileType::Water) {
                        if (step == delta) {
                            quad.texRect = {tm.waterCliff.coord[0], tm.waterCliff.coord[1], 16, 16};
                        } else {
                            quad.texRect = {tm.hardCliff.coord[0], tm.hardCliff.coord[1], 16, 16};
                        }
                    } else if (typeBelow == TileType::Stairs) {
                        if (hasLeftHardCliff) {
                            quad.texRect = {tm.shadowedStairs.coord[0], tm.shadowedStairs.coord[1], 16, 16};
                        } else {
                            quad.texRect = {tm.stairs.coord[0], tm.stairs.coord[1], 16, 16};
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
                
                bool hasTopCell = false;
                if (startR > 0 && parseTileType(typeGrid[startR - 1][c]) == TileType::Land && levelGrid[startR - 1][c] == levelGrid[startR][c]) {
                    hasTopCell = true;
                }

                auto addShadowPixels = [&](int sr, int sc, int layer, const std::vector<std::string>& pattern) {
                    RenderTile shadowTile;
                    shadowTile.layer = layer;
                    shadowTile.gridRow = sr;
                    shadowTile.gridCol = sc;
                    
                    for (int py = 0; py < pattern.size(); ++py) {
                        for (int px = 0; px < pattern[py].size(); ++px) {
                            if (pattern[py][px] == 'x') {
                                TileQuadInfo pixelQuad;
                                pixelQuad.isTextured = false;
                                pixelQuad.color = sf::Color(0, 0, 0, 100);
                                pixelQuad.renderOffset = {static_cast<float>(px), static_cast<float>(py)};
                                pixelQuad.renderSize = {1.f, 1.f};
                                shadowTile.quads.push_back(pixelQuad);
                            }
                        }
                    }
                    if (!shadowTile.quads.empty()) {
                        data.shadowTiles.push_back(shadowTile);
                    }
                };

                if (hasTopCell) {
                    std::vector<std::string> topPattern = {
                        "................",
                        "................",
                        "................",
                        "................",
                        "................",
                        "................",
                        "................",
                        "................",
                        "................",
                        "................",
                        "................",
                        "................",
                        "................",
                        "xxx.............",
                        "xxx.............",
                        "xxx............."
                    };
                    addShadowPixels(startR - 1, c, levelGrid[startR - 1][c] + 1, topPattern);
                }

                for (int currR = startR; currR <= endR; ++currR) {
                    if (currR == endR) { // bottom cell
                        std::vector<std::string> botPattern = {
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xx..............",
                            "x...............",
                            "................",
                            "................",
                            "................",
                            "................"
                        };
                        addShadowPixels(currR, c, levelGrid[currR][c] + 1, botPattern);
                    } else { // middle cells and top cell
                        std::vector<std::string> midPattern = {
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xxx.............",
                            "xxx............."
                        };
                        addShadowPixels(currR, c, levelGrid[currR][c] + 1, midPattern);
                    }
                }
            } else {
                r++;
            }
        }
    }

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

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            TileType ct = parseTileType(typeGrid[r][c]);
            if (ct == TileType::Void || ct == TileType::Water) continue;
            
            uint8_t mask = 0;

            auto canConnect = [&](TileType current, TileType neighbor, int dirIdx) {
                // dirIdx: 0=Up, 1=Right, 2=Down, 3=Left
                if (neighbor == TileType::Void || neighbor == TileType::Water) return false;
                
                // Bridge constraints
                if (current == TileType::VerticalBridge && (dirIdx == 1 || dirIdx == 3)) return false;
                if (current == TileType::HorizontalBridge && (dirIdx == 0 || dirIdx == 2)) return false;
                
                if (neighbor == TileType::VerticalBridge && (dirIdx == 1 || dirIdx == 3)) return false;
                if (neighbor == TileType::HorizontalBridge && (dirIdx == 0 || dirIdx == 2)) return false;

                return true;
            };

            if (r > 0) {
                TileType nt = parseTileType(typeGrid[r-1][c]);
                if (canConnect(ct, nt, 0)) {
                    if (levelGrid[r][c] == levelGrid[r-1][c] || ct == TileType::Stairs || nt == TileType::Stairs) mask |= 1;
                }
            }
            if (c < cols - 1) {
                TileType nt = parseTileType(typeGrid[r][c+1]);
                if (canConnect(ct, nt, 1)) {
                    if (levelGrid[r][c] == levelGrid[r][c+1] || ct == TileType::Stairs || nt == TileType::Stairs) mask |= 2;
                }
            }
            if (r < rows - 1) {
                TileType nt = parseTileType(typeGrid[r+1][c]);
                if (canConnect(ct, nt, 2)) {
                    if (levelGrid[r][c] == levelGrid[r+1][c] || ct == TileType::Stairs || nt == TileType::Stairs) mask |= 4;
                }
            }
            if (c > 0) {
                TileType nt = parseTileType(typeGrid[r][c-1]);
                if (canConnect(ct, nt, 3)) {
                    if (levelGrid[r][c] == levelGrid[r][c-1] || ct == TileType::Stairs || nt == TileType::Stairs) mask |= 8;
                }
            }
            data.walkableGrid[r][c] = mask;
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
