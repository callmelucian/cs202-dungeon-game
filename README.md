# CS202 Dungeon Game: Echoes of the Ashen Vault

This is the GitHub repository for Nguyễn Tấn Minh and Châu Tấn Phát's CS202 game project.

## Setup Instructions

Follow these steps to set up, build, and run the project on your local machine.

### 1. Prerequisites
Ensure you have the following installed and configured in your system `PATH`:
* **C++ Compiler:** Supporting C++20 (e.g., GCC/MinGW-w64 14.2.0 or newer).
* **CMake:** Version 3.25 or newer.
* **SFML 3.0.x:** Download the package matching your compiler/architecture from the [SFML Downloads Page](https://www.sfml-dev.org/download.php).

### 2. Configure SFML Path
Open [CMakeLists.txt](CMakeLists.txt) and locate the following line:
```cmake
set(SFML_ROOT "C:/Program Files/SFML/SFML-3.0.2")
```
Update `"C:/Program Files/SFML/SFML-3.0.2"` to the directory where you extracted your local SFML 3.0.x installation.

### 3. Build the Project
Open a terminal in the root directory of the repository and execute:

```powershell
# Configure the build files
cmake -S . -B build -G "MinGW Makefiles"

# Compile the project
cmake --build build
```

> [!NOTE]
> The build system automatically copies the required SFML dynamic link libraries (`.dll` files) to the output binary folder upon building, so you don't need to do it manually.

### 4. Run the Game
Once built, run the executable generated in the output directory:
```powershell
.\build\game.exe
```

## Notes for teammates

- Implementation disclipines:
  - Variable naming: `camelCase` for variables/functions, `CamelCases` for classes.
  - Avoid using `auto` in classes/modules with high dependency.
- Header files: All definition in `.hpp` files and implementation in `.cpp` files. The header file should contain a brief comment explaining the usage of all objects defined within.
- Linking: Use the header guards instead of `#pragma once`, the keyword for header guards is the filename. Example of header guards:

```cpp
#ifndef GAME_HPP
#define GAME_HPP

// implementation of src/core/game.hpp

#endif // GAME_HPP
```
- Folder structure: Every new files created must follow the proposed folder structure below, if any new files not included in the plan are needed, update the folder structure and notify the team.
- Branching: For big modules with low-depency develop it on a separate branch and create a pull request later.
- Register files into CMake: To register a new `.cpp` file into CMake so it compiles everytime we run `cmake --build build`, go to [CMakeLists.txt](CMakeLists.txt), find the `add_executable` function starting from line 17. Then add the absolute path of the new `.cpp` file onto the list.
- Memory Safety & RAII: No raw `new` and `delete`. Use smart pointers (`std::unique_ptr` by default). Wrap external resources in RAII wrapper classes or load them via `AssetManager`.
- Naming Conventions: File names are lowercase-with-hyphens. Classes/structs are PascalCase. Functions/variables are camelCase. Class member variables do not contain any prefixes. Constants/Macros are UPPERCASE_WITH_UNDERSCORES.
- Class Design & Code Modularity: Use `const` for getter methods. Use virtual destructors for polymorphic classes. Use `.hpp` and `#ifndef` guards (not `#pragma once`). Keep `#include` statements restricted to `.cpp` files when possible by using forward declarations. Avoid `auto` in high dependency modules. Use STL algorithms instead of manual loops.

## Save Data Format (`savegame.json`)

The game uses `nlohmann/json` to serialize the `RunState` struct. The resulting save file is human-readable and stores enums as their underlying integer values.

```json
{
    "currentLevel": 1,
    "currentChamber": 1,
    "echoOutcomes": {
        "0": 0,
        "1": 0,
        "2": 0,
        "3": 0,
        "4": 0
    },
    "echoesStolen": 0,
    "playerHP": 100.0,
    "activeForm": 0,
    "wraithbladeMomentum": 0.0,
    "voidcasterMomentum": 0.0,
    "ironshellMomentum": 0.0,
    "collectTimeReduction": 1.0,
    "special1MomentumThreshold": 50.0,
    "special2MomentumThreshold": 100.0,
    "foretellActive": false,
    "foretellPhase1": false
}
```

**Enum Mapping Reference:**
- **`activeForm`**: `0` = WRAITHBLADE, `1` = VOIDCASTER, `2` = IRONSHELL
- **`echoOutcomes` Keys**: `0` = MARROW, `1` = HOLLOW_BELL, `2` = CLARITY_SHARD, `3` = RESONANCE_CORE, `4` = OBSIDIAN_KEY
- **`echoOutcomes` Values**: `0` = UNCOLLECTED, `1` = COLLECTED, `2` = STOLEN

## Proposed folder structure

```
cs202-dungeon-game/
├── CMakeLists.txt                 # Master CMake project file
├── README.md                      # Project documentation
├── documents/                     # Architecture & Planning documents (already exists)
│   ├── CS202-FinalProject_SuperMario.md
│   ├── architecture.puml
│   ├── drafting.md
│   ├── game-description.md
│   └── implementation-plan.md
│
├── assets/                        # External game assets loaded at runtime
│   ├── fonts/                     # Font files (e.g., Arial, custom pixel fonts)
│   ├── textures/                  # Character sprites, chamber tiles, UI elements
│   ├── sounds/                    # Combat, collection, navigation sound effects
│   ├── music/                     # Ambient tracks & boss theme
│   └── maps/                      # Chamber configs (XML, JSON, or CSV files loaded by MapLoader)
│
└── src/                           # Source Code Directory
    ├── main.cpp                   # Application entry point
    │
    ├── core/                      # Core Game Loop & State Management
    │   ├── game.hpp / game.cpp             # Game Singleton
    │   ├── state-manager.hpp / state-manager.cpp # Manages GameState stack
    │   ├── game-state.hpp / game-state.cpp   # Abstract base state
    │   └── states/                         # Concrete game states
    │       ├── main-menu-state.hpp / main-menu-state.cpp
    │       ├── gameplay-state.hpp / gameplay-state.cpp
    │       ├── pause-state.hpp / pause-state.cpp
    │       ├── game-over-state.hpp / game-over-state.cpp
    │       ├── setting-state.hpp / setting-state.cpp
    │       └── victory-state.hpp / victory-state.cpp
    │
    ├── global-settings/                 # Engine Subsystem Managers (Singletons)
    │   ├── setting-manager.hpp / setting-manager.cpp # Manages settings & keybinds
    │   ├── asset-manager.hpp / asset-manager.cpp     # Resource cache (textures, fonts)
    │   ├── sound-manager.hpp / sound-manager.cpp     # Controls sound and music
    │   ├── save-load-manager.hpp / save-load-manager.cpp # Serialization of RunState
    │   ├── map-loader.hpp / map-loader.cpp           # Parses map & wave configuration files
    │   └── color-palette-manager.hpp / color-palette-manager.cpp # ColorPalette, ColorPaletteObserver, ColorPaletteManager singleton
    │
    ├── entities/                  # Entity Hierarchy & Action Strategy
    │   ├── character.hpp / character.cpp           # Abstract base character
    │   ├── player.hpp / player.cpp                 # Player class (owns forms)
    │   ├── status-effect.hpp / status-effect.cpp   # Status effect base
    │   ├── player-form.hpp / player-form.cpp       # Strategy base
    │   │
    │   ├── forms/                 # Player Forms (Strategy Pattern)
    │   │   ├── wraithblade-form.hpp / wraithblade-form.cpp
    │   │   ├── voidcaster-form.hpp / voidcaster-form.cpp
    │   │   └── ironshell-form.hpp / ironshell-form.cpp
    │   │
    │   ├── effects/               # Status Effects (Decorator/Strategy)
    │   │   ├── burned-effect.hpp / burned-effect.cpp
    │   │   ├── paralyzed-effect.hpp / paralyzed-effect.cpp
    │   │   └── slowed-effect.hpp / slowed-effect.cpp
    │   │
    │   └── enemies/               # Enemies & AI (Factory Pattern)
    │       ├── enemy.hpp / enemy.cpp               # Abstract enemy base
    │       ├── enemy-factory.hpp / enemy-factory.cpp # Instantiates enemies from enums
    │       ├── waterlogged-scribe.hpp / waterlogged-scribe.cpp
    │       ├── bone-sprinter.hpp / bone-sprinter.cpp
    │       ├── shard-soldier.hpp / shard-soldier.cpp
    │       ├── siege-wraith.hpp / siege-wraith.cpp
    │       ├── choir-husk.hpp / choir-husk.cpp
    │       ├── resonant-cantor.hpp / resonant-cantor.cpp
    │       ├── hushed-stalker.hpp / hushed-stalker.cpp
    │       ├── mirror-bearer.hpp / mirror-bearer.cpp
    │       ├── void-shunter.hpp / void-shunter.cpp
    │       ├── sarcophagus-warden.hpp / sarcophagus-warden.cpp
    │       └── boss-malachar.hpp / boss-malachar.cpp
    │
    ├── chambers/                  # Rooms and Level Layouts (Factory Pattern)
    │   ├── chamber.hpp / chamber.cpp               # Abstract base chamber
    │   ├── chamber-factory.hpp / chamber-factory.cpp # Instantiates chambers from config
    │   ├── protect-chamber.hpp / protect-chamber.cpp
    │   ├── prevent-chamber.hpp / prevent-chamber.cpp
    │   ├── gauntlet-chamber.hpp / gauntlet-chamber.cpp
    │   ├── mid-chamber.hpp / mid-chamber.cpp
    │   └── boss-chamber.hpp / boss-chamber.cpp
    │
    ├── economy/                   # Items & Echoes (Observer Pattern)
    │   ├── item.hpp / item.cpp                     # Abstract collectible base
    │   ├── echo-fragment.hpp / echo-fragment.cpp     # Collectible shard
    │   ├── echo-observer.hpp                        # Observer interface
    │   ├── echo.hpp / echo.cpp                     # Subject base class for Echoes
    │   └── echoes/                                 # Concrete Echoes
    │       ├── clarity-shard.hpp / clarity-shard.cpp
    │       ├── marrow-echo.hpp / marrow-echo.cpp
    │       ├── hollow-bell.hpp / hollow-bell.cpp
    │       ├── resonance-core.hpp / resonance-core.cpp
    │       └── obsidian-key.hpp / obsidian-key.cpp
    │
    ├── ui/                        # UI Framework & Game HUD
    │   ├── base/                                   # Core UI abstractions
    │   │   ├── component.hpp / component.cpp       # Abstract Component base (sf::Drawable + ColorPaletteObserver)
    │   │   └── text.hpp / text.cpp                 # SFML sf::Text wrapper
    │   ├── containers/                             # Layout containers
    │   │   ├── container.hpp / container.cpp       # Abstract Container base (holds children)
    │   │   ├── vertical-box.hpp / vertical-box.cpp # Vertical layout with Left/Center/Right alignment
    │   │   └── horizontal-box.hpp / horizontal-box.cpp # Horizontal layout with Top/Middle/Bottom alignment
    │   └── widgets/                               # Interactive leaf widgets
    │       ├── button.hpp / button.cpp             # Clickable button with hover/press states
    │       ├── slider.hpp / slider.cpp             # Draggable value slider
    │       └── text-input.hpp / text-input.cpp     # Single-line text field with cursor
    │
    └── utils/                     # Shared Structs, Helpers & Math
        ├── collision-solver.hpp / collision-solver.cpp # Stateless static helper for geometry/physics
        ├── run-state.hpp                            # Serialisable state struct (data only)
        └── common.hpp                              # Shared Enums (EchoType, FormType, etc.)
```