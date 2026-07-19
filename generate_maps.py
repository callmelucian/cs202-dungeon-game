import json
import os

# Create the output directory
os.makedirs('maps/level1', exist_ok=True)

# Helper function to generate a walled grid with optional obstacles
def make_grid(w, h, obstacles=None):
    grid = []
    for y in range(h):
        row = []
        for x in range(w):
            if x == 0 or x == w - 1 or y == 0 or y == h - 1:
                row.append(1) # Outer wall
            else:
                row.append(0) # Walkable Floor
        grid.append(row)
        
    # Inject 2x2 or custom sized obstacles (value = 2)
    if obstacles:
        for (ox, oy, ow, oh) in obstacles:
            for y in range(oy, oy + oh):
                for x in range(ox, ox + ow):
                    if 0 < y < h-1 and 0 < x < w-1:
                        grid[y][x] = 2
    return grid

# Chamber 1: Protect Chamber (Drowned Archive)
c1 = {
    'chamberType': 'ProtectChamber',
    'width': 20,
    'height': 20,
    'grid': make_grid(20, 20, [(5, 5, 2, 2), (15, 15, 2, 2)]),
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
    'grid': make_grid(30, 15, [(10, 5, 2, 5)]),
    'waves': [
        {'enemyType': 'BONE_SPRINTER', 'count': 5, 'spawnDelay': 1.0}
    ]
}

# Chamber 3: Gauntlet Chamber (Collapsed Barracks)
c3 = {
    'chamberType': 'GauntletChamber',
    'width': 25,
    'height': 25,
    'grid': make_grid(25, 25, [(10, 10, 5, 5)]),
    'waves': [
        {'enemyType': 'SHARD_SOLDIER', 'count': 5, 'spawnDelay': 0.0},
        {'enemyType': 'WATERLOGGED_SCRIBE', 'count': 4, 'spawnDelay': 0.0},
        {'enemyType': 'BONE_SPRINTER', 'count': 6, 'spawnDelay': 0.0}
    ]
}

# Write configs to files
with open('maps/level1/chamber1.json', 'w') as f: json.dump(c1, f, indent=2)
with open('maps/level1/chamber2.json', 'w') as f: json.dump(c2, f, indent=2)
with open('maps/level1/chamber3.json', 'w') as f: json.dump(c3, f, indent=2)

print("Successfully generated Level 1 JSON layouts in maps/level1/")
