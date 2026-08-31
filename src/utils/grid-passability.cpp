#include "grid-passability.hpp"

static std::string getBridgeAt(int r, int c, const std::vector<std::vector<std::string>>& bridgeGrid) {
    if (bridgeGrid.empty() || r < 0 || r >= static_cast<int>(bridgeGrid.size())) return ".";
    if (c < 0 || c >= static_cast<int>(bridgeGrid[r].size())) return ".";
    const std::string& b = bridgeGrid[r][c];
    return b.empty() ? "." : b;
}

bool GridPassability::isCliffFace(int r, int c, const std::vector<std::vector<int>>& levelGrid, const std::vector<std::vector<std::string>>& typeGrid) {
    int rows = static_cast<int>(levelGrid.size());
    int cols = levelGrid.empty() ? 0 : static_cast<int>(levelGrid[0].size());
    if (r <= 0 || r >= rows || c < 0 || c >= cols) return false;

    // Ground tile directly below an elevated tile (where northern tile is higher and not stairs)
    if (levelGrid[r - 1][c] > levelGrid[r][c]) {
        if (!typeGrid.empty() && r - 1 < static_cast<int>(typeGrid.size()) && c < static_cast<int>(typeGrid[r - 1].size())) {
            if (typeGrid[r - 1][c] != "S") return true;
        } else {
            return true;
        }
    }
    return false;
}

bool GridPassability::canConnect(int r1, int c1, int r2, int c2, int dirIdx, const GridData& grid) {
    int rows = static_cast<int>(grid.levelGrid.size());
    int cols = grid.levelGrid.empty() ? 0 : static_cast<int>(grid.levelGrid[0].size());

    // Out of bounds check
    if (r1 < 0 || r1 >= rows || c1 < 0 || c1 >= cols) return false;
    if (r2 < 0 || r2 >= rows || c2 < 0 || c2 >= cols) return false;

    // Row 0 is the decorative wall-front strip — never walkable
    if (r1 == 0 || r2 == 0) return false;

    std::string b1 = getBridgeAt(r1, c1, grid.bridgeGrid);
    std::string b2 = getBridgeAt(r2, c2, grid.bridgeGrid);
    bool hasBridge1 = (b1 == "H" || b1 == "V");
    bool hasBridge2 = (b2 == "H" || b2 == "V");

    const std::string& t1 = grid.typeGrid[r1][c1];
    const std::string& t2 = grid.typeGrid[r2][c2];

    // Void ("0") and Water ("W") are impassable unless bridged
    if (!hasBridge2 && (t2 == "0" || t2 == "W")) return false;
    if (!hasBridge1 && (t1 == "0" || t1 == "W")) return false;

    // Cliff faces are impassable unless bridged
    if (!hasBridge2 && isCliffFace(r2, c2, grid.levelGrid, grid.typeGrid)) return false;

    // Bridge directional constraints:
    // "V" bridge allows vertical (dirIdx 0=Up, 2=Down); "H" allows horizontal (dirIdx 1=Right, 3=Left)
    if (b1 == "V" && (dirIdx == 1 || dirIdx == 3)) return false;
    if (b1 == "H" && (dirIdx == 0 || dirIdx == 2)) return false;
    if (b2 == "V" && (dirIdx == 1 || dirIdx == 3)) return false;
    if (b2 == "H" && (dirIdx == 0 || dirIdx == 2)) return false;

    // Elevation level matching rule:
    // Different height levels can ONLY connect if at least one of the two cells is Stairs ("S")
    if (!hasBridge1 && !hasBridge2) {
        int lvl1 = grid.levelGrid[r1][c1];
        int lvl2 = grid.levelGrid[r2][c2];
        if (lvl1 != lvl2) {
            if (t1 != "S" && t2 != "S") {
                return false;
            }
        }
    }

    return true;
}
