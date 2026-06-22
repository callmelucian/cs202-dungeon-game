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
#ifndef GAME
#define GAME

// implementation of src/core/game.hpp

#endif // GAME
```
- Folder structure: Every new files created must follow the proposed folder structure below, if any new files not included in the plan are needed, update the folder structure and notify the team.
- Branching: For big modules with low-depency develop it on a separate branch and create a pull request later.
- Register files into CMake: To register a new `.cpp` file into CMake so it compiles everytime we run `cmake --build build`, go to [CMakeLists.txt](CMakeLists.txt), find the `add_executable` function starting from line 17. Then add the absolute path of the new `.cpp` file onto the list.

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
    │   ├── Game.hpp / Game.cpp             # Game Singleton
    │   ├── StateManager.hpp / StateManager.cpp # Manages GameState stack
    │   ├── GameState.hpp / GameState.cpp   # Abstract base state
    │   └── states/                         # Concrete game states
    │       ├── MainMenuState.hpp / MainMenuState.cpp
    │       ├── GameplayState.hpp / GameplayState.cpp
    │       ├── PauseState.hpp / PauseState.cpp
    │       ├── GameOverState.hpp / GameOverState.cpp
    │       └── VictoryState.hpp / VictoryState.cpp
    │
    ├── global-setings/                  # Engine Subsystem Managers (Singletons)
    │   ├── SettingManager.hpp / SettingManager.cpp # Manages settings & keybinds
    │   ├── AssetManager.hpp / AssetManager.cpp     # Resource cache (textures, fonts)
    │   ├── SoundManager.hpp / SoundManager.cpp     # Controls sound and music
    │   ├── SaveLoadManager.hpp / SaveLoadManager.cpp # Serialization of RunState
    │   └── MapLoader.hpp / MapLoader.cpp           # Parses map & wave configuration files
    │
    ├── entities/                  # Entity Hierarchy & Action Strategy
    │   ├── Character.hpp / Character.cpp           # Abstract base character
    │   ├── Player.hpp / Player.cpp                 # Player class (owns forms)
    │   │
    │   ├── forms/                 # Player Forms (Strategy Pattern)
    │   │   ├── PlayerForm.hpp / PlayerForm.cpp     # Strategy base
    │   │   ├── WraithbladeForm.hpp / WraithbladeForm.cpp
    │   │   ├── VoidcasterForm.hpp / VoidcasterForm.cpp
    │   │   └── IronshellForm.hpp / IronshellForm.cpp
    │   │
    │   ├── effects/               # Status Effects (Decorator/Strategy)
    │   │   ├── StatusEffect.hpp / StatusEffect.cpp # Status effect base
    │   │   ├── BurnedEffect.hpp / BurnedEffect.cpp
    │   │   ├── ParalyzedEffect.hpp / ParalyzedEffect.cpp
    │   │   └── SlowedEffect.hpp / SlowedEffect.cpp
    │   │
    │   └── enemies/               # Enemies & AI (Factory Pattern)
    │       ├── Enemy.hpp / Enemy.cpp               # Abstract enemy base
    │       ├── EnemyFactory.hpp / EnemyFactory.cpp # Instantiates enemies from enums
    │       ├── WaterloggedScribe.hpp / WaterloggedScribe.cpp
    │       ├── BoneSprinter.hpp / BoneSprinter.cpp
    │       ├── ShardSoldier.hpp / ShardSoldier.cpp
    │       ├── SiegeWraith.hpp / SiegeWraith.cpp
    │       ├── ChoirHusk.hpp / ChoirHusk.cpp
    │       ├── ResonantCantor.hpp / ResonantCantor.cpp
    │       ├── HushedStalker.hpp / HushedStalker.cpp
    │       ├── MirrorBearer.hpp / MirrorBearer.cpp
    │       ├── VoidShunter.hpp / VoidShunter.cpp
    │       ├── SarcophagusWarden.hpp / SarcophagusWarden.cpp
    │       └── BossMalachar.hpp / BossMalachar.cpp
    │
    ├── chambers/                  # Rooms and Level Layouts (Factory Pattern)
    │   ├── Chamber.hpp / Chamber.cpp               # Abstract base chamber
    │   ├── ChamberFactory.hpp / ChamberFactory.cpp # Instantiates chambers from config
    │   ├── ProtectChamber.hpp / ProtectChamber.cpp
    │   ├── PreventChamber.hpp / PreventChamber.cpp
    │   ├── GauntletChamber.hpp / GauntletChamber.cpp
    │   ├── MidChamber.hpp / MidChamber.cpp
    │   └── BossChamber.hpp / BossChamber.cpp
    │
    ├── economy/                   # Items & Echoes (Observer Pattern)
    │   ├── Item.hpp / Item.cpp                     # Abstract collectible base
    │   ├── EchoFragment.hpp / EchoFragment.cpp     # Collectible shard
    │   ├── EchoObserver.hpp                        # Observer interface
    │   ├── Echo.hpp / Echo.cpp                     # Subject base class for Echoes
    │   └── echoes/                                 # Concrete Echoes
    │       ├── ClarityShard.hpp / ClarityShard.cpp
    │       ├── MarrowEcho.hpp / MarrowEcho.cpp
    │       ├── HollowBell.hpp / HollowBell.cpp
    │       ├── ResonanceCore.hpp / ResonanceCore.cpp
    │       └── ObsidianKey.hpp / ObsidianKey.cpp
    │
    ├── ui/                        # User Interface Elements
    │   └── HUD.hpp / HUD.cpp                       # Concrete EchoObserver, displays stats
    │
    └── utils/                     # Shared Structs, Helpers & Math
        ├── CollisionSolver.hpp / CollisionSolver.cpp # Stateless static helper for geometry/physics
        ├── RunState.hpp                            # Serialisable state struct (data only)
        └── Common.hpp                              # Shared Enums (EchoType, FormType, etc.)
```