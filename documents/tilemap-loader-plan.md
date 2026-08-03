# 2.5D Tilemap Loader Specification

This document details the architectural design and implementation plan for the **2.5D Tilemap Loader** module. It specifies how top-view 2D chamber descriptions are transformed into 2.5D elevated rendering layouts with autotiled terrain, cliffs, shadows, and bridges.

---

## 0. Global rules

- If no matching assets can be found, throw an exception instead of finding for a fallback.
- If the provided grid in the chamber configuration file is invalid, also throw an exception instead of trying to fix it.

---

## 1. Overview & Architectural Goals

The dungeon game represents chambers using a **2.5D top-down elevated perspective**. The loader takes a 2D logical representation of a chamber (containing cell types and height levels) and synthesizes a fully rendered multi-level 2.5D map layout using texture assets defined in `assets/animations/tile-map.json`.

---

## 2. Input Chamber Grid Schema (`assets/maps/level-x/chamber-x.json`)

Each chamber file provides two aligned 2D matrices of identical dimensions:

1. **`type-grid`**: Encodes the logical cell terrain type:
   - `"0"`: Void (empty space / bottomless pit)
   - `"L"`: Land (walkable ground)
   - `"S"`: Stairs (on the land below an elevated ground, when rendering, treat this cell as land).
   - `"W"`: Water (liquid hazard / body of water)
   - `"V"`: Vertical Bridge (connects two same-level Land cells vertically)
   - `"H"`: Horizontal Bridge (connects two same-level Land cells horizontally)

2. **`level-grid`**: Encodes the height level of each cell:
   - `0`: Level of Void
   - $x \ge 1$: Integer elevation level of the chamber terrain

3. **`playerSpawn`**: An array `[row, col]` of 2 integers specifying the 0-indexed cell coordinates (row `i`, column `j`) where the player spawns. This cell MUST be a Land (`"L"`) cell with elevation level $\ge 1$.

---

## 3. Class API Prototype

The module is implemented as a singleton factory class.

---

## 4. Autotiling & Neighbor Matching Algorithm

### 4.1 8-Neighbor Grid Indexing
Each cell evaluates its 8 surrounding neighbors in **row-major order** (skipping the center cell):

$$\begin{matrix} 1 & 2 & 3 \\ 4 & \cdot & 5 \\ 6 & 7 & 8 \end{matrix}$$

### 4.2 Connected Component Decomposition
1. Render each elevation level independently from lowest level to highest level.
2. For a given level $L$, gather all cells with elevation $\ge L$. Label higher-level cells and non-water cells as `Land` (they will be overwritten by higher-level rendering passes later).
3. Decompose cells into **connected components** using 4-connectivity (sharing an edge).
4. Any cell outside the current connected component is treated as **Void** (`V`).

### 4.3 Quadrant Assembly ($8 \times 8$ Sub-tiles)
A $16 \times 16$ grid tile is constructed either from a single `FILLED` $16 \times 16$ asset or by combining four $8 \times 8$ quadrant sub-tiles:

- **Top-Left (TL)** quadrant matches neighbors **(1, 2, 4)**
- **Top-Right (TR)** quadrant matches neighbors **(2, 3, 5)**
- **Bottom-Left (BL)** quadrant matches neighbors **(4, 6, 7)**
- **Bottom-Right (BR)** quadrant matches neighbors **(5, 7, 8)**

### 4.4 Neighbor Encoding Alphabet
The `neighbors` string in the asset JSON uses a tile-type specific alphabet:

- **`LAND` tiles**:
  - `E`: Elevated land, this is prioritized higher than normal land, if we cannot match `E`, try to match `L`.
  - `-`: Anything other than elevated land.
  - `L`: Land / water
  - `V`: Void
- **`WATER` tiles**:
  - `W`: Water (same type)
  - `L`: Land / non-water (different type)
  - `?`: Can match with anything

For example, if a water pond is load like this

```
LLLL
LWWL
LLLL
```

Then for the first `W` tile, you must look for the following combination:
- Top-left quadrant: `LLL`
- Top-right quadrant: `LLW`
- Bottom-left quadrant: `LLL`
- Bottom-right quadrant: `WLL`

A similar idea is used to determine the tile for the second `W`, this will draw the edges of the lake correctly and for the land tiles, all of the quadrants that is adjencent to the lake is simply `LLL`.

---

## 5. Layering & 2.5D Elevation System

### 5.1 Rendering Sequence
1. Iterate from lowest elevation level to highest elevation level.
2. On each level, render from top row to bottom row.
3. For each cell, render base tiles, followed by cliffs directly underneath (allowing lower rows to properly overlap cliff bases).

### 5.2 Cliff Generation Logic
When a cell at $(r, c)$ has a higher elevation level than the adjacent cell directly below it at $(r+1, c)$, calculate the elevation drop $\Delta = \text{level}(r, c) - \text{level}(r+1, c)$:

- **Below cell is Void**: Assemble a vertical column of $(\Delta - 1)$ `semi-hard-cliff` tiles followed by $1$ `soft-cliff` tile. If $\Delta = 1$, place a single `semi-soft-cliff` tile.
- **Below cell is Water**: Assemble a vertical column of $(\Delta - 1)$ `hard-cliff` tiles followed by $1$ `water-cliff` tile.
- **Below cell is Land but not stairs-type**: Assemble a vertical column of $\Delta$ `hard-cliff` tiles.
- **Below cell is Land and is stairs-type**: Assemble a vertical column of $\Delta$ `stairs` tiles, or `shadowed-stairs` tiles if there is  column of hard cliffs to the left of it.

### 5.3 Shadowing

For each contiguous vertical column of land cells whose immediate left neighbor is elevated land on the 2.5D map (plus an optional single land cell directly above the column):
- For the bottom cell, create overlaying transparent dark pixels at the following cells:
```
xxx.............
xxx.............
xxx.............
xxx.............
xxx.............
xxx.............
xxx.............
xxx.............
xxx.............
xxx.............
xx..............
x...............
................
................
................
................
```

- For the middle cells (if there is no optional single land cell directly above the column, this should also include the top cell), create overlaying transparent dark pixels at the following cells:
```
xxx.............
xxx.............
xxx.............
xxx.............
xxx.............
xxx.............
xxx.............
xxx.............
xxx.............
xxx.............
xxx.............
xxx.............
xxx.............
xxx.............
xxx.............
xxx.............
```

- For the top optional single land cell directly above the column, create overlaying transparent dark pixels at the following cells
```
................
................
................
................
................
................
................
................
................
................
................
................
................
xxx.............
xxx.............
xxx.............
```

### 5.4 Overlays & Bridges
Bridge tiles (`vertical-bridge` of size $16 \times 20$ and `horizontal-bridge` of size $20 \times 16$) and obstacle overlays are placed on top of the assembled base tiles.

---

## 6. Asset Atlas Integration (`assets/animations/tile-map.json`)

### 6.1 Tile Textures (`tile-textures`)
Each entry contains:
- `coord`: Array `[x, y]` specifying top-left pixel coordinates in the tilesheet.
- `position`: One of `"TL"`, `"TR"`, `"BL"`, `"BR"`, `"ANY"` ($8 \times 8$ quadrant), or `"FILLED"` ($16 \times 16$ tile).
- `neighbors`: String of 3 characters (quadrants) or 8 characters (`FILLED`).
- `texture` *(optional)*: Integer variant index.
  - Variant `0` has a selection weight of $\frac{9}{10}$.
  - Positive variant indices $1\dots x$ share uniform selection weight $\frac{1}{10x}$.

### 6.2 Cliff Assets (`cliffs`)

Contains explicit coordinates for cliffs and stairs.

### 6.3 Overlay Assets (`overlays`)

Contains coordinates and dimensions for bridges.

---

# 7. Navigating in 2.5D map

With the new map rendering logic, we have to adjust the logic for determining obstacles on the 2.5D walkable terrain:

- Tiles of the same level are considered connected, we cannot walk from a lower tile to a higher tile without stairs, we cannot also step on cliffs.
- We cannot step on lakes.
- Bridges only allow movement along their orientation (e.g., Vertical bridges only allow Up/Down traversals, blocking side-to-side paths).

A possible way to do this is simply construct a grid matching the 2.5D map and determine for each side (shared between 1 cells), is it a walkable side, enemy steering logic should follow this grid. Thus, this information is fixed and the logic for navigating in the 2.5D map should **not** change depending on the character's position.

---

## 8. Gameplay Integration

### 8.1 Player Coordinate Alignment
When movement input is released, the character position automatically snaps to the nearest integer grid coordinate to ensure smooth navigation through narrow 1-cell corridors.

### 8.2 Enemy Pathfinding & Steering
Enemy navigation utilizes a grid graph constructed from the 2.5D walkable terrain:
1. Build an adjacency graph of all walkable cells considering height elevation transitions and bridges.
2. Execute Breadth-First Search (BFS) to calculate the shortest path to the player's current cell position.
3. Derive movement vectors along the computed path.
