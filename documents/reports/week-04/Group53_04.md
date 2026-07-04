# Weekly Report Content

## General Information

- Group ID: 53
- Group name: Echoes of the Ashen Vault.
- Project name: Echoes of the Ashen Vault (Dungeon Game).
- Date range: 29.06.2026 - 04.07.2026

## Tasks completed this week

Since this is the first weekly report, we will include all tasks from the first 4 weeks.

### 25125027 - Nguyễn Tấn Minh:

#### Week 01 & 02

- Create [detailed game description](game-description.md) and [OOP architecture](architecture.puml).
- Setup project, folder structure, and CMake and link dependencies.

#### Week 03

- Implement `Game` singleton wrapper (`getInstance()`, `run()`, game loop, window setup).
- Implement `SettingManager` singleton storing/loading game volumes, resolution, and control binds.
- Implement stack-based `StateManager` (`pushState()`, `popState()`, `changeState()`).
- Implement abstract `GameState` class interface.
- Create concrete states: `MainMenuState` (Start, Load, Exit buttons), `GameplayState` (black window placeholder), `PauseState` (overlay), `GameOverState` (receives `EndingType` enum to display Ending A, B, or C).
- Planning on easy-to-use UI framework classes and color palette management.

#### Week 04

- Implement complete easy-to-use UI framework that supports `UI::FlexBox` for automatic position computation.
- Implement abstract `Character` base class (position, velocity, HP/maxHP, moveSpeed, baseDamage, attackRange, attackRate, sprite, `vector<unique_ptr<StatusEffect>> activeEffects`, `takeDamage()`, `tickStatusEffects()`, `getEffectiveMoveSpeed()`).
- Implement `Player` class inheriting from `Character`. The player owns all three `PlayerForm` instances at all times (`unique_ptr` members) and holds a raw `PlayerForm*` pointing at whichever is active. Also track `switchCooldownTimer`, `inMidChamber` flag (suspends the 4.0s cooldown when true), and the shared HP pool sized to the active form's `maxHP`.
- Implement abstract `PlayerForm` Strategy base class with `attack()`, `useSpecial1()`, `useSpecial2()`, `onMomentumGainHit()`, `onMomentumGainDamageTaken()`, `addMomentum()`, `resetMomentum()`, and `setSpecial1Threshold()` (called at runtime by `RunState` when Hollow Bell is collected).
- Code `WraithbladeForm` (Speed 7.0, MaxHP 100, Dmg 12, Range 1.5 melee, Rate 2/s; Momentum +6 per hit landed, +0.4 per Wraithblade-equivalent HP lost).
- Code `VoidcasterForm` (Speed 5.0, MaxHP 70, Dmg 22, Range 12 piercing, Rate 1/s; Momentum +8 per far-range hit, +4 per additional pierced enemy, +0.4 per HP lost).
- Code `IronshellForm` (Speed 2.5, MaxHP 160, Dmg 6, Range 1.0 cleave, Rate 1/s; Momentum +1.2 per Ironshell-equivalent HP lost, +3 per hit landed).
- Planning for seminar & overall plan updates.

### 25125061 - Châu Tấn Phát:

#### Week 01 & 02

- Create [implementation plan](task-list.md) as a detailed 7-week tasks lists, each week including tasks for 2 developrs and tasks involving AI agents. Note that week count from week 3, so the first week of the plan is week 3 of the course.

#### Week 03

- Implement `AssetManager` singleton caching textures, fonts.
- Implement `SoundManager` singleton controlling background music streams and sound buffers.
- Implement `SaveLoadManager` singleton with `saveGame(RunState&)` and `loadGame() → RunState` interfaces; keep it decoupled from map data (that is `MapLoader`'s job).
- Define the `RunState` struct (fields: `currentLevel`, `currentChamber`, `map<EchoType, EchoOutcome> echoOutcomes`, `playerHP`, `activeForm`, per-form Momentum floats, `collectTimeReduction`, `special1MomentumThreshold`, `special2MomentumThreshold`,  `foretellActive`, `foretellPhase1`). This struct is used by every subsequent system and must exist before Week 2 work begins.
- Integrate default UI font and basic game sound assets (menu navigation sounds, placeholder music).

#### Week 04

- Bind WASD/D-pad to player velocity.
- Implement `CollisionSolver` static utility class: `aabbIntersect()`, `circleIntersect()`, `resolveAABB()`, `lineIntersectsRect()`, and `lineIntersectsCircle()` (the last two are needed for Voidcaster's wall-piercing Special 1 in Week 5).
- Implement the switch key binds (1, 2, 3) to swap active `PlayerForm` strategies. On switch: reset the outgoing form's Momentum to 0, carry the incoming form's frozen Momentum, apply the HP conversion formula `new_HP = (current_HP / old_max) × new_max`, and start the 4.0s `switchCooldownTimer`. Block switching if `switchCooldownTimer > 0` and `inMidChamber` is false. *(Note: HP conversion removed, `inMidChamer` is determined by `Player::isSwitchCooldownEnabled`)*
- Implement 4.0s switch cooldown timer that blocks swapping but does not inhibit movement or combat.

## AI Usage Declaration

- [**Claude.** Anthropic, accessed 19/06/2026. Purpose: Write a detailed game description with specific characters' stats from brief game description and rulesets.](https://claude.ai/share/f005033d-1f05-4450-91b9-43eaa16ce4c4)
- **Gemini.** Google, via Antigravity, accessed 11:23 AM 19/06/2026. Purpose: Design a detailed implementation plan given the game description and architecture.
- Other tasks done by AI agents using Gemini model, via Antigravity:
  - Boilerplate CMake file configuration.
  - Standard C++ singleton template class structure.
  - Window initialization and rendering loop boilerplate.
  - `EndingType`, `EchoType`, `EchoOutcome`, `FormType`, `ChamberType`, and `EnemyType` enum definitions (one shared header; all other classes include it).
  - Implement reusable UI framework classes under folder `ui` inside namespace `UI` (`Component`, `Text`, `Button`, `Slider`, `Container`, `FlexBox<Axis>`, `HorizontalBox`, `VerticalBox`, `TextInput`) & `ColorPalette` struct, `ColorPaletteManager` singleton, and `ColorPaletteObserver` interface.
  - Standard vector math utility functions (normalization, dot product, distance checks).
  - Static class attributes and getters/setters for `Character` and `PlayerForm`.
  - 2D AABB intersection checking code.

## Issues

No issues encountered during this week.