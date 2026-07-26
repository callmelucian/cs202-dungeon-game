# Map Creation Guide

Echoes of the Ashen Vault uses a JSON-based campaign and chamber system to make custom storyline creation easy.

## 1. Campaign Format

The `maps/campaign.json` file defines the overarching structure of the run:

```json
{
  "levels": [
    {
      "levelIndex": 1,
      "name": "The Outer Wards",
      "chambers": [
        { "file": "maps/level1/chamber1.json" },
        { "file": "maps/level1/chamber2.json" }
      ]
    }
  ]
}
```
You can add as many chambers to a level as you want. The final chamber of each level will automatically advance the player to the next level in the array upon completion.

## 2. Chamber Format

Individual chamber files define the grid layout, chamber type, and enemy waves. Example:

```json
{
  "chamberType": "ProtectChamber",
  "width": 20,
  "height": 20,
  "playerSpawn": {
    "x": 2.5,
    "y": 2.5
  },
  "grid": [
    [1, 1, 1, 1, 1],
    [1, 0, 0, 0, 1],
    [1, 1, 1, 1, 1]
  ],
  "waves": [
    {
      "enemyType": "WATERLOGGED_SCRIBE",
      "count": 3,
      "spawnDelay": 2.0
    }
  ]
}
```

### 2.0 Player Spawn
Use `"playerSpawn"` (object `{ "x": float, "y": float }` or array `[x, y]`) to explicitly define where the character starts (in grid coordinates). If omitted, the game automatically scans the grid and places the player on the first available open ground tile (`0`).

### 2.1 Grid Integers

The map grid uses integers to represent terrain types:
- `0` = Floor (Walkable)
- `1` = Outer Wall (Impassable)
- `2` = Lake / Water (Impassable)
- `4` = Elevated Floor (Walkable)
- `5` = Stairs (Walkable)
- `6` = Cliff Face (Impassable)

### 2.2 Valid Chamber Types
Set `"chamberType"` to one of the following strings:
- `"ProtectChamber"`: Defend an Echo in the center.
- `"PreventChamber"`: Stop carriers from escaping.
- `"GauntletChamber"`: Survive waves of enemies with no delay.
- `"MidChamber"`: Safe zone, no enemies, allows free form-switching.
- `"TestChamber"`: Debug/Placeholder chamber.

### 2.3 Valid Enemy Types (Waves)
Use these strings for `"enemyType"` in the `"waves"` array:
- `"WATERLOGGED_SCRIBE"`
- `"SHARD_SOLDIER"`
- `"BONE_SPRINTER"`
- `"SHARD_WRAITH"`
- `"SIEGE_WRAITH"`
- `"CHOIR_HUSK"`
- `"RESONANT_CANTOR"`
- `"HUSHED_STALKER"`
- `"MIRROR_BEARER"`
- `"VOID_SHUNTER"`
- `"SARCOPHAGUS_WARDEN"`
- `"BOSS_MALACHAR"`
