import json
import os

# Create the output directory
os.makedirs('assets/maps/level-1', exist_ok=True)

# Helper function to generate a walled grid with optional obstacles
def make_grid(w, h, obstacles=None):
    grid = []
    for y in range(h):
        row = []
        for x in range(w):
            if x == 0 or x == w - 1 or y == 0 or y == h - 1:
                row.append("0") # Outer wall / Void
            else:
                row.append("L") # Walkable Land
        grid.append(row)
        
    if obstacles:
        for (ox, oy, ow, oh) in obstacles:
            for y in range(oy, oy + oh):
                for x in range(ox, ox + ow):
                    if 0 < y < h-1 and 0 < x < w-1:
                        grid[y][x] = "W" # Water hazard
    return grid

def make_level_grid(w, h):
    grid = []
    for y in range(h):
        row = []
        for x in range(w):
            if x == 0 or x == w - 1 or y == 0 or y == h - 1:
                row.append(0)
            else:
                row.append(1)
        grid.append(row)
    return grid

# Chamber 1: Protect Chamber (Drowned Archive)
c1 = {
    'chamberType': 'ProtectChamber',
    'width': 20,
    'height': 20,
    'playerSpawn': [2, 2],
    'type-grid': make_grid(20, 20, [(5, 5, 2, 2), (15, 15, 2, 2)]),
    'level-grid': make_level_grid(20, 20),
    'waves': [
        {'enemyType': 'WATERLOGGED_SCRIBE', 'count': 3, 'spawnDelay': 2.0},
        {'enemyType': 'SHARD_SOLDIER', 'count': 2, 'spawnDelay': 5.0}
    ]
}

# Chamber 2: Prevent Chamber (Bone Corridor)
c2 = {
    'chamberType': 'PreventChamber',
    'width': 30,
    'height': 15,
    'playerSpawn': [1, 1],
    'type-grid': make_grid(30, 15, [(10, 5, 2, 5)]),
    'level-grid': make_level_grid(30, 15),
    'waves': [
        {'enemyType': 'BONE_SPRINTER', 'count': 5, 'spawnDelay': 1.0}
    ]
}

# Chamber 3: Gauntlet Chamber (Collapsed Barracks)
c3 = {
    'chamberType': 'GauntletChamber',
    'width': 25,
    'height': 25,
    'playerSpawn': [1, 1],
    'type-grid': make_grid(25, 25, [(10, 10, 5, 5)]),
    'level-grid': make_level_grid(25, 25),
    'waves': [
        {'enemyType': 'SHARD_SOLDIER', 'count': 5, 'spawnDelay': 0.0},
        {'enemyType': 'WATERLOGGED_SCRIBE', 'count': 4, 'spawnDelay': 0.0},
        {'enemyType': 'BONE_SPRINTER', 'count': 6, 'spawnDelay': 0.0}
    ]
}

# Write configs to files
with open('assets/maps/level-1/chamber-1.json', 'w') as f: json.dump(c1, f, indent=2)
with open('assets/maps/level-1/chamber-2.json', 'w') as f: json.dump(c2, f, indent=2)
with open('assets/maps/level-1/chamber-3.json', 'w') as f: json.dump(c3, f, indent=2)

print("Successfully generated Level 1 JSON layouts in assets/maps/level-1/")
