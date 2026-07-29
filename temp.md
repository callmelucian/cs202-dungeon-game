Implement the following:

# Chamber intro

When entering each chamber, the following happen:
- All assets are rendered and then frozen, the camera is initiated zoomed out.
- A title text given by an `UI::Container titleContainer` (temporarily containing a title text) is displayed for 2s, then disappear.
- The camera is zoomed in.
- Characters are unfreezed and the game is played.

# Echo collector timer

- Implement a collector timer, using `EnemyHealthBar`.

# Wave spawner?

- Investigate why level 1 chamber 1 has no enemy despited being loaded in the `json` configuration file.
- Also make sure the wave spawner of all other chambers work properly if the enemies are loaded in `json` configuration file.

# Scan for duplicated code