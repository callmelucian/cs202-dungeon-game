# Echoes of the Ashen Vault — Project Implementation Plan & OOP Checklist

This document contains the 6-week detailed project plan and OOP architecture design for the 2-member C++ development team of **Echoes of the Ashen Vault**. 

A complete class diagram modeling these dependencies is available in [architecture.puml](file:///d:/Code/CS202%20Lab/cs202-dungeon-game/documents/architecture.puml).

---

## 1. OOP Architecture & Design Patterns Overview

### Core Object-Oriented Principles
* **Abstraction**: High-level abstract base classes (`Character`, `PlayerForm`, `Chamber`, `Item`, `StatusEffect`) declare interface contracts (`update()`, `draw()`, `takeDamage()`) without exposing concrete implementations.
* **Inheritance**: Models system relationships. `Player` and `Enemy` inherit from `Character`. Specific chamber types (e.g., `ProtectChamber`) inherit from `Chamber`.
* **Polymorphism**: Enables runtime dynamic dispatch. A single `std::vector<std::unique_ptr<Enemy>>` processes all active enemies polymorphically, and `PlayerForm*` swaps dynamically to alter player behaviors.
* **Encapsulation**: Keeps attributes `private` or `protected` and provides access control through setters/getters (e.g. `Character::takeDamage()`, `Player::gainMomentum()`).

### Integrated Design Patterns
1. **Singleton Pattern**: Applies to `Game`, `AssetManager`, `SoundManager`, and `SaveLoadManager` to guarantee single instances controlling critical engine services.
2. **State Pattern**: The stack-based `StateManager` manages active `GameState` implementations (`MainMenuState`, `GameplayState`, etc.).
3. **Strategy Pattern**: The player controls a dynamic `PlayerForm` reference, swapping between `WraithbladeForm`, `VoidcasterForm`, and `IronshellForm` at runtime.
4. **Factory Pattern**: `EnemyFactory` and `ChamberFactory` dynamically instantiate concrete classes based on type enums or configurations.
5. **Observer Pattern**: `Echo` acts as a subject notifying registered `EchoObserver` instances (e.g., the HUD/UI in `GameplayState`) of power percentage updates.

---

## 2. 6-Week Granular Checklist & Task Breakdown

### Week 1: Core Engine, States & Asset Management
* **Developer A (Core Engine & States)**
  - [ ] Configure CMakeLists.txt to fetch and link dependencies (SFML/Raylib, etc.).
  - [ ] Implement `Game` singleton wrapper (`getInstance()`, `run()`, game loop, window setup).
  - [ ] Implement stack-based `StateManager` (`pushState()`, `popState()`, `changeState()`).
  - [ ] Implement abstract `GameState` class interface.
  - [ ] Create concrete states: `MainMenuState` (Start, Load, Exit buttons), `GameplayState` (black window placeholder), `PauseState` (overlay).
* **Developer B (Resource & System Managers)**
  - [ ] Implement `AssetManager` singleton caching text overlays, fonts, and textures.
  - [ ] Implement `SoundManager` singleton controlling background music streams and sound buffers.
  - [ ] Implement `SaveLoadManager` singleton wrapper with file read/write interfaces.
  - [ ] Integrate default UI font and basic game sound assets (menu navigation sounds, placeholder music).
* **AI-Assisted Tasks (Boilerplate & Utilities)**
  - [ ] Boilerplate CMake file configuration.
  - [ ] Standard C++ singleton template class structure.
  - [ ] Window initialization and rendering loop boilerplate.
* **Week 1 Deliverable**: A compilable game window that boots into a working Main Menu. Clicking "Start" transitions to the gameplay screen; pressing Escape opens/closes the pause state; clicking "Exit" performs a clean shutdown.

---

### Week 2: Entity System & Player Forms
* **Developer A (Player & Strategy Forms)**
  - [ ] Implement abstract `Character` base class (managing position, velocity, base stats, and sprite rendering).
  - [ ] Implement `Player` class inheriting from `Character`, maintaining form-specific momentum variables.
  - [ ] Implement abstract `PlayerForm` Strategy pattern base class.
  - [ ] Code `WraithbladeForm` (close range, high speed, momentum increments on hit/damage).
  - [ ] Code `VoidcasterForm` (long range, piercing, momentum increments on far-range hits).
  - [ ] Code `IronshellForm` (low speed, high HP, defensive momentum increments on damage taken).
* **Developer B (Movement, Input & Collisions)**
  - [ ] Bind WASD/D-pad to player velocity.
  - [ ] Implement 2D AABB and circle collision solvers against static wall obstacles.
  - [ ] Implement the switch key binds (1, 2, 3) to swap active `PlayerForm` strategies.
  - [ ] Implement the HP conversion percentage formula: `new_HP = (current_HP / old_max) * new_max` on switch.
  - [ ] Implement 4.0s switch cooldown timer that blocks swapping but does not inhibit movement/combat.
* **AI-Assisted Tasks (Boilerplate & Utilities)**
  - [ ] Standard vector math utility functions (normalization, dot product, distance checks).
  - [ ] Static class attributes and getters/setters for `Character` and `PlayerForm`.
  - [ ] 2D AABB intersection checking code.
* **Week 2 Deliverable**: A controllable player character who can navigate a test grid, collide with walls, and switch between 3 forms using hotkeys. The HUD displays the active form and shows health converting dynamically based on maximum HP contributions.

---

### Week 3: Status Effects & Basic Enemies
* **Developer A (Status Effects & Combat Hitboxes)**
  - [ ] Implement `StatusEffect` abstract base class.
  - [ ] Implement `BurnedEffect` (ticks DOT damage based on dealer's attack).
  - [ ] Implement `ParalyzedEffect` (40% action-fail chance checker).
  - [ ] Implement `SlowedEffect` (reduces move speed to 70%).
  - [ ] Set up status effect timers and auto-removal ticks inside `Character::update()`.
  - [ ] Implement player melee and ranged attack hitboxes that scale by active form parameters.
* **Developer B (Enemy Base & Basic AI)**
  - [ ] Implement `Enemy` base class inheriting from `Character`.
  - [ ] Implement `EnemyFactory` mapping IDs/enums to concrete enemy constructions.
  - [ ] Code simple steering AI behaviors: **Seek** (chases player position) and **Evade** (moves away from threat).
  - [ ] Implement `WaterloggedScribe` (slow, simple melee attack).
  - [ ] Implement `ShardSoldier` (standard melee speed/damage).
* **AI-Assisted Tasks (Boilerplate & Utilities)**
  - [ ] Timer-based status effect tick boilers.
  - [ ] Standard steering behavior algorithms (Seek, Flee, Obstacle Avoidance).
  - [ ] Class constructors for basic enemies via the factory.
* **Week 3 Deliverable**: A playable scene containing basic spawned enemies that chase the player. The player can attack, deal damage, and apply status effects (visible via floating labels or status icons above the entities).

---

### Week 4: Chamber Logic & Fragment Economy
* **Developer A (Chamber System & Protect Mode)**
  - [ ] Implement abstract `Chamber` base class.
  - [ ] Implement `ChamberFactory` to load and build maps.
  - [ ] Code **Protect Chamber** layout and logic (radius check, proximity collection timer).
  - [ ] Implement Ironshell's "Damage Redirect" skill (redirects 100% of Echo damage to Serin when standing on the Echo).
  - [ ] Implement Wraithblade knockback push (pushes enemies away from Echo position by 4 units).
* **Developer B (Economy, Prevent Mode & Save/Load Serialization)**
  - [ ] Implement `Item` base, `EchoFragment` physics, and `Echo` Observer pattern base class.
  - [ ] Implement fragment multiplier logic by active form (knockback collision, pierce-kills, slowed-death doubles).
  - [ ] Implement **Prevent Chamber** logic (real vs. decoy carrier checks: real carriers stagger on hit, decoys do not).
  - [ ] Code the file reader to parse level maps and spawn coordinates from structured external files.
  - [ ] Implement Save/Load serialization variables inside `SaveLoadManager`.
* **AI-Assisted Tasks (Boilerplate & Utilities)**
  - [ ] JSON/CSV map reading parser functions.
  - [ ] Standard implementation of the Observer pattern for `Echo` and UI observers.
  - [ ] Basic properties for fragment spawning physics.
* **Week 4 Deliverable**: A fully playable Level 1 Chamber 1 (Protect) and Chamber 2 (Prevent). Players can collect fragments pre- and mid-collection, defend the Echo, trigger form-specific fragment bonuses, and trigger state saving/loading.

---

### Week 5: Advanced Chambers, AI & Boss Setup
* **Developer A (Gauntlet, Mid-Chamber & Boss AI)**
  - [ ] Implement **Gauntlet Chamber** logic (tracking multiple waves with 0s spawn delay).
  - [ ] Implement **Mid-Chamber** mechanics (free-switching, +15 momentum increment on exit).
  - [ ] Create `BossMalachar` class and Phase 1 behavior (ranged void bolts cycle, summons 2 Shard Wraiths).
  - [ ] Add boss phase transition check at 75% HP.
* **Developer B (Advanced Enemy Mechanics)**
  - [ ] Implement `SiegeWraith` (melee + death AOE explosion).
  - [ ] Implement `ChoirHusk` (call-response attack sync) and `ResonantCantor` (slow pulse).
  - [ ] Implement `HushedStalker` (invisible state, noise-generation tracking in Chamber 3).
  - [ ] Implement `MirrorBearer` (decoys, real carrier flicker tell).
  - [ ] Implement `VoidShunter` (charging vectors, knockback physics) and `SarcophagusWarden` (guard stance).
* **AI-Assisted Tasks (Boilerplate & Utilities)**
  - [ ] Wave spawner timers and count checks.
  - [ ] Invisibility shader/sprite blending code for Hushed Stalkers.
  - [ ] Exploding entity radius checker algorithms.
* **Week 5 Deliverable**: Complete playable Level 2 and Level 3 chambers featuring advanced enemy interactions (invisible stalkers, mirror decoys, exploding siege wraiths). The player completes the Level 3, Chamber 4 "Sarcophagus Approach" and enters the boss arena.

---

### Week 6: Boss Phases, Serialization & Polish
* **Developer A (Boss Phases 2, 3, 4 & Endings)**
  - [ ] Implement Phase 2 modifiers (Marrow HP regeneration, Obsidian Key blinking).
  - [ ] Implement Phase 3 collapsing platforms (6 floating platform layout, platform sunder attack).
  - [ ] Implement Phase 4 shrinking platforms and Soul Lance attack.
  - [ ] Code the ending checker to select Ending A, B, or C depending on collected vs. stolen Echoes.
* **Developer B (HUD, Audio, Save/Load & QA)**
  - [ ] Complete `SaveLoadManager` to write/load active game progress cleanly.
  - [ ] Design game HUD (health bars, active status effects, active form indicator, 3 persistent momentum meters).
  - [ ] Integrate sound effects (form switching, sword swings, projectile fire, enemy hit, fragment pickup).
  - [ ] Conduct debugging, playtesting, balance adjustments, and clean up memory leaks.
* **AI-Assisted Tasks (Boilerplate & Utilities)**
  - [ ] HUD layout positioning coordinates.
  - [ ] Save data serialization structures.
  - [ ] End-of-game screens rendering boilerplate.
* **Week 6 Deliverable**: A fully complete and polished game. Players can play from Level 1, progress through all levels, save/load, defeat Malachar (through all 4 phases), and view Ending A, B, or C.

---

## 3. Weekly Deliverable Verification Checklist

To confirm project health, at the end of each week, the development team must verify the following:

* [ ] **Week 1 Deliverable**: Compile and run the project. Boot into the main menu, select "Start Game" to switch to gameplay, press Escape to pause, and click "Quit" to ensure it returns 0.
* [ ] **Week 2 Deliverable**: Move player. Switch forms (1, 2, 3). Check that HP converts correctly (e.g. 50/100 Wraithblade becomes 80/160 Ironshell) and that the 4-second swap cooldown is enforced.
* [ ] **Week 3 Deliverable**: Spawn basic enemies. Attack them to verify hitboxes register, damage is subtracted, and status icons (Burned/Slowed/Paralyzed) display correctly above target.
* [ ] **Week 4 Deliverable**: Play Chamber 1 (Protect) and Chamber 2 (Prevent). Check that collecting fragments increases Echo Power, standing on the Echo redirects damage to the player, and hitting real carriers staggers them.
* [ ] **Week 5 Deliverable**: Play Gauntlet waves, verify 0s delay transition, and test that all advanced enemy actions function (Invisible Stalkers are visible in Ironshell's aura; Shard Wraiths explode).
* [ ] **Week 6 Deliverable**: Fight Malachar. Confirm Phase transitions occur at correct HP fractions, platforms collapse in Phase 3, platforms shrink in Phase 4, and Ending A, B, or C displays based on results.
