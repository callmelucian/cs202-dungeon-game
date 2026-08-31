#ifndef GRID_PASSABILITY_HPP
#define GRID_PASSABILITY_HPP

#include <vector>
#include <string>
#include <cstdint>

struct GridData {
    const std::vector<std::vector<std::string>>& typeGrid;
    const std::vector<std::vector<int>>& levelGrid;
    const std::vector<std::vector<std::string>>& bridgeGrid;
};

class GridPassability {
public:
    // Returns true if character/unit can physically move from cell (r1, c1) to neighbor (r2, c2)
    // in direction dirIdx (0=Up, 1=Right, 2=Down, 3=Left).
    static bool canConnect(int r1, int c1, int r2, int c2, int dirIdx, const GridData& grid);

    // Returns true if cell (r, c) is a cliff face tile (ground tile directly below a higher elevation tile)
    static bool isCliffFace(int r, int c, const std::vector<std::vector<int>>& levelGrid, const std::vector<std::vector<std::string>>& typeGrid);
};

#endif // GRID_PASSABILITY_HPP
