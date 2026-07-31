# Plans for new tilemap loader

Since the current tilemap loader module is very fragile, this documents acts as the new tilemap loader implementation plan, **fully written by a human**.

## Implementation prototype

I want a singleton class `TilemapLoader` that acts as a factory that takes in a map grid and decides which tile to take.

The class should at least include the following function:
- Private function `int tilePicker (TileType current, const std::vector<TileType> &neighbor)` where `neighbor` is always a vector of 8 elements. It takes in the current tile and the neighboring tiles' type, and return the ID of the tile that should be rendered from the given tilesheet.
- Public function `std::vector<std::vector<int>> loadMap (const std::vector<std::vector<int>> &grid)`. It takes in `grid` as the inputting raw description of the chamber and return the grid with the exact tile from the tilesheet to be rendered

## Mechanism

### Assets

Assets will be rendered from `assets/textures/dungeon-tiles.png` and `assets/animations/tile-map.json` (atlas), in the following format:

- Under section `main-tiles`:
  - The name of each tile will be `[NAME]-[NEIGHBORS]` where `[NAME]` is the name of the current tile and `[NEIGHBORS]` is the string of 8 characters describing the neighbors (from smaller to larger row, then from smaller to larger column).
  - Each tile will be given 2 attributes `x`, `y`, the top-left corner coordinate of the tile.
  - All tiles within `main-tiles` have the same size, given by `width` and `height`.
- Under section `layer-tiles`:
  - The name of each tile will be `[NAME]` as the appearance of layer tiles do not depend on neighbors.
  - Each till will be given 4 attributes `x`, `y`, `width`, `height`, please align the tile so that its center fit the center of the main tiles.

### Tile picker

For the 8-character strings, I would like to decode each cell as follows:
- `?`: everything can fit in.
- `x`: tiles of another type can fit in.
- `o`: tiles of the same type can fit in.
- `0`: land tiles can fit in.
- `1`: water tiles can fit in.

When the tile is on the edge, null neighbor is considered different type.

Some exceptions:
- `DOOR` is the same type as `CLIFF-FRONT`
- For `LAND` tile, if a `DOOR` type is placed on top of it, `DOOR` can be considered as the same type.

### Encoding

The inputting grid can be represented by the following values:
- `0`: land
- `1`: water
- `2`: cliff (only appear on the top row)
- `3`: door (only appear once on the top row)

Please check of the encoding is valid before loading the tilemap

## Other modifications

- The exit of a chamber is always a door, which is opened after the quest is done, when the user is standing nearby, a button pop-ups and ask the player if he wants to exit the chamber.
- The empty space around the grid when user tries to zoom out should be water (same logic as water tiling), as if all cells are `1` by default.
- User can not zoom out above the grid, as the top row is always the cliff/door.