# Echoes of the Ashen Vault — Project Implementation Plan & OOP Checklist

## Week 1: Core Engine, States & Asset Management (22.06 -> 27.06)

* **Developer A (Core Engine & States)**
  - [x] Configure CMakeLists.txt to fetch and link dependencies (SFML/Raylib, etc.).
  - [x] Implement `Game` singleton wrapper (`getInstance()`, `run()`, game loop, window setup).
  - [x] Implement `SettingManager` singleton storing/loading game volumes, resolution, and control binds.
  - [x] Implement stack-based `StateManager` (`pushState()`, `popState()`, `changeState()`).
  - [x] Implement abstract `GameState` class interface.
  - [x] Create concrete states: `MainMenuState` (Start, Load, Exit buttons), `GameplayState` (black window placeholder), `PauseState` (overlay), `GameOverState` (receives `EndingType` enum to display Ending A, B, or C).
  - [x] Planning on easy-to-use UI framework classes and color palette management.

* **Developer B (Resource & System Managers)**
  - [x] Implement `AssetManager` singleton caching textures, fonts.
  - [x] Implement `SoundManager` singleton controlling background music streams and sound buffers.
  - [x] Implement `SaveLoadManager` singleton with `saveGame(RunState&)` and `loadGame() → RunState` interfaces; keep it decoupled from map data (that is `MapLoader`'s job).
  - [x] Define the `RunState` struct (fields: `currentLevel`, `currentChamber`, `map<EchoType, EchoOutcome> echoOutcomes`, `playerHP`, `activeForm`, per-form Momentum floats, `collectTimeReduction`, `special1MomentumThreshold`, `special2MomentumThreshold`,  `foretellActive`, `foretellPhase1`). This struct is used by every subsequent system and must exist before Week 2 work begins.
  - [x] Integrate default UI font and basic game sound assets (menu navigation sounds, placeholder music).

* **AI Agent Tasks**
  - [x] Boilerplate CMake file configuration.
  - [x] Standard C++ singleton template class structure.
  - [x] Window initialization and rendering loop boilerplate.
  - [x] `EndingType`, `EchoType`, `EchoOutcome`, `FormType`, `ChamberType`, and `EnemyType` enum definitions (one shared header; all other classes include it).
  - [x] Implement reusable UI framework classes under folder `ui` inside namespace `UI` (`Component`, `Text`, `Button`, `Slider`, `Container`, `FlexBox<Axis>`, `HorizontalBox`, `VerticalBox`, `TextInput`) & `ColorPalette` struct, `ColorPaletteManager` singleton, and `ColorPaletteObserver` interface.

* **Deliverable**: A compilable game window that boots into a working Main Menu. Clicking "Start" transitions to the gameplay screen; pressing Escape opens/closes the pause state; clicking "Exit" performs a clean shutdown. Confirm `RunState` struct compiles and all shared enums (`EchoType`, `FormType`, `EchoOutcome`, `EndingType`, `ChamberType`, `EnemyType`) are accessible across translation units.

---

## Week 2: Entity System & Player Forms (28.06 -> 04.07)

* **Developer A (Player & Strategy Forms)**
  - [x] Implement abstract `Character` base class (position, velocity, HP/maxHP, moveSpeed, baseDamage, attackRange, attackRate, sprite, `vector<unique_ptr<StatusEffect>> activeEffects`, `takeDamage()`, `tickStatusEffects()`, `getEffectiveMoveSpeed()`).
  - [x] Implement `Player` class inheriting from `Character`. The player owns all three `PlayerForm` instances at all times (`unique_ptr` members) and holds a raw `PlayerForm*` pointing at whichever is active. Also track `switchCooldownTimer`, `inMidChamber` flag (suspends the 4.0s cooldown when true), and the shared HP pool sized to the active form's `maxHP`.
  - [x] Implement abstract `PlayerForm` Strategy base class with `attack()`, `useSpecial1()`, `useSpecial2()`, `onMomentumGainHit()`, `onMomentumGainDamageTaken()`, `addMomentum()`, `resetMomentum()`, and `setSpecial1Threshold()` (called at runtime by `RunState` when Hollow Bell is collected).
  - [x] Code `WraithbladeForm` (Speed 7.0, MaxHP 100, Dmg 12, Range 1.5 melee, Rate 2/s; Momentum +6 per hit landed, +0.4 per Wraithblade-equivalent HP lost).
  - [x] Code `VoidcasterForm` (Speed 5.0, MaxHP 70, Dmg 22, Range 12 piercing, Rate 1/s; Momentum +8 per far-range hit, +4 per additional pierced enemy, +0.4 per HP lost).
  - [x] Code `IronshellForm` (Speed 2.5, MaxHP 160, Dmg 6, Range 1.0 cleave, Rate 1/s; Momentum +1.2 per Ironshell-equivalent HP lost, +3 per hit landed).
  - [x] Planning for seminar & overall plan updates.
  - [x] Weekly Report.

* **Developer B (Movement, Input & Collisions)**
  - [x] Bind WASD/D-pad to player velocity.
  - [x] Implement `CollisionSolver` static utility class: `aabbIntersect()`, `circleIntersect()`, `resolveAABB()`, `lineIntersectsRect()`, and `lineIntersectsCircle()` (the last two are needed for Voidcaster's wall-piercing Special 1 in Week 5).
  - [x] Implement the switch key binds (1, 2, 3) to swap active `PlayerForm` strategies. On switch: reset the outgoing form's Momentum to 0, carry the incoming form's frozen Momentum, apply the HP conversion formula `new_HP = (current_HP / old_max) × new_max`, and start the 4.0s `switchCooldownTimer`. Block switching if `switchCooldownTimer > 0` and `inMidChamber` is false. *(Note: HP conversion removed, `inMidChamer` is determined by `Player::isSwitchCooldownEnabled`)*
  - [x] Implement 4.0s switch cooldown timer that blocks swapping but does not inhibit movement or combat.

* **AI Agent Tasks**
  - [x] Standard vector math utility functions (normalization, dot product, distance checks).
  - [x] Static class attributes and getters/setters for `Character` and `PlayerForm`.
  - [x] 2D AABB intersection checking code.

* **Deliverable**: A controllable player character who can navigate a test grid, collide with walls, and switch between 3 forms using hotkeys. The HUD displays the active form and shows health converting dynamically based on maximum HP contributions. Verify the 4.0s swap cooldown is enforced and that switching sets the outgoing form's Momentum to 0 and resumes the incoming form's frozen Momentum value. Temporarily set `inMidChamber = true` and confirm the cooldown is suspended.

---

## Week 3: SpriteSheet System, Status Effects, Basic Enemies & Base Stat Model (05.07 -> 11.07)

* **Developer A (SpriteSheet/Animation System, Stats & Status Effects)**
  - [ ] Design and source/create sprite assets: player character (3 forms × idle/walk/attack frames), basic enemy sprites (WaterloggedScribe, ShardSoldier, BoneSprinter), tilemap tiles (stone floor, water, walls, barriers).
  - [x] Implement `Stats` and `StatModifier` structs. Add flat `defense` stat to forms (Wraithblade: 15, Voidcaster: 5, Ironshell: 35).
  - [x] Implement the Defense mitigation formula in `Character::calculateMitigatedDamage(rawAmount)`: `mitigated = rawAmount * 100 / (100 + defense)`, rounded, clamped to min 1. Wire into `Character::takeDamage(rawAmount)` as the single entry point for all hits.
  - [x] Implement decoupled visual Observer pattern: `CharacterObserver` interface (`onStateChanged`, `onDamaged`, `onDefeated`) and `CharacterAnimator` class (managing sprite, current animation key, hit flash timer, observers callbacks). Integrate with `Character::draw()`.
  - [x] Implement `SpriteSheet` class: load texture atlas, define frame rects, support multi-row layouts for different animation states.
  - [x] Implement `Animation` class: frame sequencing, timing (`frameDuration`), looping/non-looping modes, `play()`, `pause()`, `reset()`, `getCurrentFrame()`.
  - [x] Refactor `Player` to use `PlayableCharacter` abstract factory (per architecture.puml): `Player` takes a `PlayableCharacter&` in constructor, stores forms in `std::map<FormType, std::unique_ptr<PlayerForm>>`, generalizes momentum to `std::map<FormType, float>`. Implement `Serin` as the concrete `PlayableCharacter` whose `createForm1/2/3()` return `WraithbladeForm`, `VoidcasterForm`, `IronshellForm`. Decouple `Player` from specific forms.
  - [x] Implement `PlayerCombatState` interface and `PlayerCombatStateMachine` (State pattern) to manage active combat states. Formulate `PlayerForm` as a concrete subclass of `PlayerForm` implementing `PlayerCombatState`.
  - [x] Implement `StatusEffect` abstract base class with `apply(Character&)`, `remove(Character&)`, `update(dt, Character&) → bool` (per architecture.puml).
  - [x] Implement `BurnedEffect`: ticks once per second for 10 seconds, dealing `0.25 × dealerBaseDamage` per tick (affected by target defense).
  - [x] Implement `ParalyzedEffect`: 10-second duration; exposes `rollMiss()` returning true 40% of the time, re-rolled per action attempt.
  - [x] Implement `SlowedEffect`: sets effective move speed to `base_speed × 0.70` via `StatModifier`; supports a `permanent` flag for zone-based slow that ignores the 10-second expiry rule.
  - [ ] Wire `Character::applyStatusEffect(unique_ptr<StatusEffect>)` and `Character::tickStatusEffects(float dt)` into `Character::update()`.
  - [ ] Implement player melee and ranged attack hitboxes that scale by active form parameters.
  - [ ] *Seminar:* Quick draft on all 3 design patterns (Decorator, Strategy, Facade).
  - [ ] Weekly Report.

* **Developer B (Enemy Base, BoneSprinter & Basic AI State)**
  - [x] Implement `Enemy` abstract class inheriting from `Character`. Add `playerRef : Player&`, `currentState : EnemyState*`, `steeringStrategy : unique_ptr<EnemySteeringStrategy>`, `attackCooldown`, and `updateState(dt, Chamber&)` method.
  - [x] Implement the **State Pattern** for enemy behavior: `EnemyState` interface and concrete classes (`IdleState`, `ChasingState`, `AttackingState`, `StaggeredState` with timed auto-revert mechanism).
  - [x] Implement the **Strategy Pattern** for enemy steering: `EnemySteeringStrategy` interface and concrete classes (`SeekStrategy`, `EvadeStrategy`). Let `ChasingState` delegate movement to the enemy's active strategy.
  - [x] Add `fragmentDropCount` and `onDeath()` abstract method to `Enemy`.
  - [x] Implement `EnemyFactory::createEnemy(EnemyType, Player&) → unique_ptr<Enemy>` (per architecture.puml).
  - [x] Implement `WaterloggedScribe` (HP 18, Dmg 5, Speed 1.5 — effective after chamber water-slow; no additional Slowed multiplier unless additionally afflicted).
  - [x] Implement `ShardSoldier` (HP 16, Dmg 6). Include a `selfHealActive` flag and `applySelfHeal(float dt)` method.
  - [x] Implement `BoneSprinter` (HP 14 carrier / HP 20 blocker, Speed 9.0 / 6.0). Include `isCarrier` flag for carrier/blocker variants.
  - [x] Add `BONE_SPRINTER` and `SHARD_WRAITH` to `EnemyType` enum in `enums.hpp`.
  - [ ] Integrate enemy sprites from Dev A's sprite assets.

* **AI Agent Tasks**
  - [ ] Timer-based status effect tick boilerplate.
  - [ ] Standard steering behavior algorithms (Seek, Flee, Obstacle Avoidance).
  - [ ] Class constructors for basic enemies via the factory.
  - [ ] SpriteSheet frame-rect calculation utilities.
  - **Prompt (Dev A provides):** *"Generate the SpriteSheet and Animation classes for SFML. SpriteSheet loads a texture atlas and stores frame rects. Animation sequences frames with configurable frameDuration and looping. Integrate with our existing Character::draw() using CharacterAnimator and CharacterObserver. Also generate StatusEffect base class and StatModifier matching architecture.puml's signature."*

* **Deliverable**: A playable scene containing animated player and basic spawned enemies (WaterloggedScribe, ShardSoldier) that chase the player. The player can attack, deal damage, and apply status effects. Verify `SlowedEffect` permanent flag works correctly. BoneSprinter basic movement is testable. Attack them to verify hitboxes register, damage is subtracted, and status icons display correctly above target. Verify that damage is correctly mitigated by target defense (Wraithblade 15, Voidcaster 5, Ironshell 35).

---

## Week 4: Chamber System, Specials Decorators & Echo/Fragment Economy (12.07 -> 18.07)

*Mid-term week, reduce tasks for both developers.*

* **Developer A (Chamber System & Tilemap Rendering)**
  - [ ] Implement abstract `Chamber` base class with `player : Player&`, `enemies : vector<unique_ptr<Enemy>>`, `items : vector<unique_ptr<Item>>`, `isCompleted`, `update(dt)`, `draw(window)`, `spawnEnemy()`, `checkCollisions()` (per architecture.puml).
  - [ ] Implement `ChamberFactory::createChamber(level, chamberIndex, Player&) → unique_ptr<Chamber>` (per architecture.puml).
  - [ ] Implement `MapLoader` static class: `loadChamber(string path) → ChamberConfig` and `loadWaves(string path) → vector<WaveConfig>`. Define `ChamberConfig` and `WaveConfig` data structs.
  - [ ] Implement basic tilemap renderer: load tile textures, render floor/wall grid from `ChamberConfig` wall rects, support water tiles for Drowned Archive.
  - [ ] Code `ProtectChamber`: collection radius 2.5 units, collection timer, `onEchoHit()` applying −8% Echo Power, `onFragmentCollected(bool midCollection)` granting +5% / +2.5% Echo Power, `checkIronshellRedirect(Player&)` redirecting 100% of incoming Echo damage to Serin when she is within 1.0 unit of the Echo, and `applyWraithbladeKnockback(Enemy*)` pushing enemies 4 units away from Echo position.
  - [ ] Code `PreventChamber`: tracks `associatedEcho` field so it can write `RunState.echoOutcomes[associatedEcho] = STOLEN` if any real carrier reaches the exit. `onCarrierHit(Enemy*, bool lethal)` triggers the 0.5s stagger animation on real carriers (non-lethal hit only); decoys show no reaction.
  - [ ] *Task from week 2:* Include `applySlowAura(vector<Enemy*>&)` as a per-tick method that applies/refreshes `SlowedEffect` on every enemy within 4.0 units — this is called by `Player::update()` whenever Ironshell is active.
  - [ ] *Seminar:* Detailed content for `Decorator` pattern (11 items) + `Strategy` pattern (11 items).
  - [ ] Weekly Report.

* **Developer B (Specials Decorators, Echo Subject/Observer & Items)**
  - [ ] Implement `SpecialAbilityState` abstract base class (Decorator / State pattern) which delegates to `innerState` and holds a `StatModifier` and duration timer.
  - [ ] Implement the 6 concrete Special Ability Decorator states:
    - Wraithblade: `WraithbladeRiftcrushState`, `WraithbladeCinderveilState`
    - Voidcaster: `VoidcasterLanceState`, `VoidcasterDetonationFieldState`
    - Ironshell: `IronshellAegisPulseState`, `IronshellVeilOfThornsState`
  - [ ] Wire these states into `Player::triggerSpecial()`, which instantiates the decorator state and enters it in `PlayerCombatStateMachine`.
  - [ ] Implement `Item` abstract base class with `position`, `bounds`, `onCollect(Player&, Chamber&)` (per architecture.puml Economy package).
  - [ ] Implement `EchoFragment` inheriting from `Item`: physics position/velocity, `value`, `update()`, `draw()`, `onCollect()`.
  - [ ] Implement `Echo` abstract Subject class: `name`, `echoPower` (float, clamped 10–100), `observers : vector<EchoObserver*>`, `takeDamage(penalty)`, `addPower(amount)`, `getPower()`, `attach(observer)`, `notify()` (per architecture.puml).
  - [ ] Implement concrete Echo subclasses: `ClarityShard`, `MarrowEcho`, `HollowBell`, `ResonanceCore`, `ObsidianKey` (per architecture.puml — each stores its specific collected/stolen effects).
  - [ ] Implement `EchoObserver` interface with `onEchoPowerChanged(float power)`. Implement `GameplayState` as a concrete observer.
  - [ ] Implement fragment multiplier logic by active form: Wraithblade knockback-wall collision → +1 extra fragment; Voidcaster pierce-kill chain → +1 per additional enemy beyond the first killed in one shot; Ironshell kill while enemy is Slowed → fragment drop doubled.
  - [ ] Implement Prevent Chamber fragment bonus: carrier kill → 3 fragments instead of 1.
  - [ ] Create Level 1 chamber layout data files (JSON/CSV) for MapLoader.
  - [ ] *Seminar:* Detailed content for `Facade` pattern (11 items) + LaTeX report.

* **AI Agent Tasks**
  - [ ] JSON/CSV map reading parser functions for `MapLoader`.
  - [ ] Standard Observer pattern boilerplate for `Echo` → `EchoObserver`.
  - [ ] Basic fragment spawning physics (velocity scatter, gravity, collection magnet).
  - [ ] Tilemap rendering grid loop boilerplate.
  - **Prompt (Dev B provides):** *"Generate the Echo Observer pattern: abstract Echo class as Subject. Generate EchoObserver interface. Also generate the SpecialAbilityState decorator base class and concrete special decorators (WraithbladeRiftcrushState, etc.) conforming to PlayerCombatState. Generate JSON parser for MapLoader to read ChamberConfig and WaveConfig structs."*

* **Deliverable**: A fully playable Level 1 Chamber 1 (Protect) and Chamber 2 (Prevent) with tilemap rendering. Players can collect fragments, defend the Echo, trigger form-specific fragment bonuses and special abilities (Riftcrush, Cinderveil, Lance, Detonation Field, Aegis Pulse, Veil of Thorns) that wrap combat states via Decorators. Echo Observer pattern keeps GameplayState in sync. Verify that mid-collection fragments grant only +2.5% Echo Power and that Ironshell's Damage Redirect correctly subtracts from Serin's HP (run through Defense mitigation). Confirm hitting real carrier triggers the stagger animation. Save and reload; verify `RunState` is restored identically.

---

## Week 5: Advanced Chambers, Level 2–3 Enemies & Level Progression (19.07 -> 25.07)

*VNOI Cup final week, reduce tasks for Developer A. More tasks shifted to Developer A from Week 6.*

* **Developer A (Gauntlet, MidChamber & Level Progression)**
  - [ ] Implement `GauntletChamber`: wave tracker, 0s gap between waves (next wave begins the instant the last enemy of the prior wave dies — no delay), `waves : vector<vector<EnemyType>>`, `currentWaveIndex`, and `applyGauntletHeal(Player&)` granting +25% Max HP heal on chamber clear. This heal triggers at the end of Level 1 Ch.3, Level 2 Ch.3, and Level 3 Ch.3.
  - [ ] Implement `MidChamber`: sets `Player::inMidChamber = true` on `onEnter()` (suspending the 4.0s switch cooldown entirely) and `false` on `onPlayerExit()`. On exit, grant the last-active form a flat +15 Momentum bonus (clamped to 100).
  - [ ] Implement level progression flow in `GameplayState: Level 1 -> Level 2 -> Level 3 -> Boss`. Wire chamber completion → next chamber transition with Mid-Chambers in between.
  - [ ] Implement chamber retry on death (restart current chamber, preserve prior chamber results per §6.2).
  - [ ] Create Level 2 and Level 3 chamber layout data files (JSON/CSV) for MapLoader.
  - [ ] Weekly Report.

* **Developer B (Advanced Enemies — Level 2 & 3)**
  - [ ] Implement `SiegeWraith` (HP 40, Dmg 10; on death calls `explode()` dealing 15 AOE damage in a 3-unit radius — can chain into other nearby enemies including other Siege Wraiths).
  - [ ] Implement `ChoirHusk` (HP 20 standard / 16 carrier / 22 guard, Dmg 7, Speed 4.0). Call-and-response: one Husk "calls," triggering `triggerCallResponse()` which gives up to 2 nearby Husks a 0.6s shared windup before all strike together. Bonus: killing 2+ Husks within the same 0.6s window grants +1 bonus fragment (tracked in chamber update).
  - [ ] Implement `ResonantCantor` (HP 50, Dmg 9, does not move). Every 5 seconds emits a pulse applying `SlowedEffect` to Serin if within 6 units. Slow refreshes on re-application rather than stacking in magnitude.
  - [ ] Implement `HushedStalker` (HP 22, Dmg 9). Invisible until 0.4s attack wind-up telegraph. `onSlowedApplied()` sets `visible = true` for the Slowed effect's duration. Kills while Slowed drop 2 fragments and do not trigger the +1-spawn noise penalty for that specific kill. The noise mechanic (`+1 spawn per Serin offensive action`, hard cap +12 total) is tracked in `ProtectChamber` (Level 3, Chamber 1) and fed to `EnemyFactory`.
  - [ ] Implement `MirrorBearer` (HP 18 carrier / 24 guard, Speed 6.5). 1 real carrier, 2 decoys (randomised per attempt). Real carrier: `isRealCarrier()` returns true, staggers visibly for 0.5s on any non-lethal hit, drops 3 fragments on death. Decoy: no knockback reaction, no stagger, `isRealCarrier()` returns false, 0 fragments, shatters into smoke on death.
  - [ ] Implement `VoidShunter` (HP 25, Dmg 11 charge-only, Speed 6.0 when charging). Wraithblade knockback (4 units) and Riftcrush can push Shunters into the Hunger Pit void (instant kill). A Shunter's charge that connects while Serin is within 2 units of the pit edge pushes her 3 units — check `CollisionSolver` for pit boundary.
  - [ ] Implement `SarcophagusWarden` (HP 28, Dmg 10, Speed 4.5). Every 6s one Warden enters Guard Stance for 2s: immune to knockback, +50% damage resistance. Voidcaster pierce shots bypass the resistance.
  - [ ] LaTeX report for `Strategy` pattern.

* **AI Agent Tasks**
  - [ ] Wave spawner timers and count checks.
  - [ ] Invisibility shader/sprite blending code for `HushedStalker`.
  - [ ] Exploding entity radius checker algorithm (reuse `CollisionSolver::circleIntersect()`).
  - [ ] Enemy sprite generation prompts for new enemy types.
  - **Prompt (Dev A provides):** *"Generate GauntletChamber and MidChamber classes inheriting from Chamber. GauntletChamber: wave spawning with 0s delay, +25% MaxHP heal on clear. MidChamber: suspend switch cooldown, +15 Momentum on exit. Also generate wave spawner utility that reads WaveConfig and spawns enemies via EnemyFactory with configurable spawn delays and positions."*

* **Deliverable**: Complete playable Level 1 (all 3 chambers + mid-chambers) with level progression flow. Level 2/3 enemy behaviors testable. Verify: `MidChamber` suspends cooldown and grants +15 Momentum on exit; `GauntletChamber` wave gap is truly 0s; Gauntlet clear grants +25% MaxHP heal. Test all advanced enemy actions. Enemy damage is correctly reduced by Serin's active form defense (mitigated via `calculateMitigatedDamage()`). Chamber retry on death restarts current chamber only.

---

## Week 6: Boss Fight, HUD & Audio (26.07 -> 01.08)

* **Developer A (Boss — All 4 Phases & BossChamber)**
  - [ ] Create `BossChamber` wrapping `BossMalachar`. `BossChamber` owns the platform layout (6 platforms, radius 3 each for Phase 3; Phase 4 shrinks each at 0.1 units/sec, floor 1.5 units). Phase and platform state are managed here; AI behaviour is delegated to `BossMalachar`.
  - [ ] Implement `BossMalachar` class with `currentPhase`, `transitionPhase(int)`, `platformSunder(Chamber&)`, `soulLance(Chamber&)` (per architecture.puml).
  - [ ] Implement Phase 1 behaviour: **Void Bolt Cycle** (3 ranged bolts, Dmg 14 each, fired 0.8s apart, 0.5s charge-glow telegraph — extended to 1.1s total if Foretell is active from a fully-intact Clarity Shard); **Summoning Burst** (spawns 2× Shard Wraith, HP 30, Dmg 8, fly toward Serin). Fixed 12-second repeating cycle. Phase 1→2 transition check at 75% HP (937.5 HP).
  - [ ] Implement `ShardWraith` enemy (HP 30, Dmg 8) — spawned by BossMalachar Phase 1.
  - [ ] Implement Phase 2 (HP ≤ 75% → HP ≤ 50%): read `RunState` to conditionally enable Marrow Echo regen (`applyMarrowRegen(float dt)`, 2% MaxHP/sec = 25 HP/sec, active through Phases 2–4) and Obsidian Key blink (`performBlink()`, every 6–9s randomised, active through Phases 2–3). Trigger Resonance Core transition burst on the 75% threshold crossing: `resonanceCoreBurst()` deals 8% of Malachar's current HP (doubled to two sequential bursts ~1s apart if Resonance Core was fully intact).
  - [ ] Implement Phase 3 (HP ≤ 50% → HP ≤ 25%): notify `BossChamber` to shatter the floor into 6 floating platforms. Add **Platform Sunder** attack every 15 seconds: telegraphs Serin's current platform, which collapses 3 seconds after telegraph (forcing relocation). Carry over Phase 2 modifiers (Marrow regen, Obsidian blink). Trigger second Resonance Core transition burst at the 50% threshold crossing.
  - [ ] Implement Phase 4 (HP ≤ 25% → HP = 0): notify `BossChamber` to begin shrinking platforms at 0.1 units/sec (floor radius 1.5). Add **Soul Lance** attack every 10 seconds: single targeted bolt, Dmg 30, 1.0s telegraph (extended to 1.6s if Foretell active). Carry over Marrow regen if active; Obsidian Key blink does **not** carry into Phase 4 (shrinking platforms replace it). Trigger third Resonance Core transition burst at the 25% threshold crossing.
  - [ ] On phase transition, read `RunState` to apply all relevant modifiers: Hollow Bell stolen → activate `reflectWardActive` (first hit every 8s reflects 20% back to Serin, Phase 1 only); Foretell from Clarity Shard collected at any Power → begin extended telegraphs in Phase 2 (or Phase 1 if fully intact).
  - [ ] Boss sprite/animation assets — design and implement multi-phase boss visuals.
  - [ ] PowerPoint presentation on `Decorator` pattern.
  - [ ] Weekly Report.

* **Developer B (HUD, Audio, Camera & Save/Load)**
  - [ ] Implement basic camera system: follow player position, smooth lerp, clamp to chamber bounds.
  - [ ] Complete `HUD` as concrete `EchoObserver`: render health bar (sized to active form's MaxHP), active status effect icons, active form indicator, 3 persistent Momentum meters (one per form, frozen meters visually distinct), switch cooldown indicator, and Echo Power bar via `onEchoPowerChanged()`.
  - [ ] Integrate sound effects (form switching, sword swings, projectile fire, enemy hit, fragment pickup, Echo collection, boss phase transitions).
  - [ ] Design and implement particle effects: fragment pickup sparkle, Echo collection glow, enemy death poof, boss phase transition bursts.
  - [ ] Wire `SaveLoadManager` to correctly serialise all `RunState` fields. Verify that loading correctly restores per-form Momentum, Echo outcomes, `special1MomentumThreshold`, and Foretell flags so the run resumes identically.

* **AI Agent Tasks**
  - [ ] HUD layout positioning coordinates and bar rendering.
  - [ ] Save data serialization structures for `RunState`.
  - [ ] Boss attack pattern timer/state-machine boilerplate.
  - [ ] Particle system basic framework.
  - **Prompt (Dev B provides):** *"Generate the HUD class implementing EchoObserver. It should render: health bar sized to active form MaxHP, 3 Momentum meters (active + 2 frozen), status effect icons, switch cooldown indicator, and Echo Power bar updated via onEchoPowerChanged(). Use SFML RectangleShape for bars and our UI::Text for labels. Also generate a basic particle emitter class for fragment pickup and death effects."*

* **Deliverable**: Boss fight is fully playable through all 4 phases. HUD displays all game state. Sound effects integrated. Camera follows player. Verify: Resonance Core bursts fire at 75%, 50%, and 25% HP thresholds; platforms collapse in Phase 3 and shrink in Phase 4; Hollow Bell reflect ward active only Phase 1; Marrow regen persists Phases 2–4; Obsidian blink stops at Phase 4. Play Gauntlet waves, verify 0s delay transition. Confirm `MidChamber` suspends the switch cooldown and grants +15 Momentum to the exit form.

---

## Week 7: Endings, Polish, Deliverables & QA (02.08 -> 08.08)

* **Developer A (Endings, SettingState & Full Game Polish)**
  - [ ] Implement ending checker in `GameplayState` after boss defeat: count `RunState.echoesStolen`. 0 stolen → `EndingType::ENDING_A_SHATTER`; 1–2 stolen → `EndingType::ENDING_B_RETREAT`; 3–5 stolen → `EndingType::ENDING_C_WARNING`. Push `GameOverState(ending)`.
  - [ ] Enhance `GameOverState` to display: narrative text and visual treatment for each ending (A: triumphant golden palette, B: somber blue-grey, C: dark red-black), Echo Power summary table (all 5 Echoes with final Power% and Collected/Stolen status), fade-in animation, "Return to Main Menu" button.
  - [ ] Fix `EndingType` enum duplication: remove from `game-over-state.hpp`, use only `enums.hpp` version.
  - [ ] Implement `SettingState` key rebinding UI (connect to `SettingManager::setKeyBinding()`).
  - [ ] Complete Level 3 Chamber 4 (Sarcophagus Approach) decoy reliquary: successful 10s collect grants +20% Max HP buff for the Final Chamber only.
  - [ ] Full end-to-end game flow testing: Level 1 → Mid → Level 2 → Mid → Level 3 → Boss → Ending.
  - [ ] Asset polish: finalize all sprite animations, ensure consistent art style.
  - [ ] **Create OOP & Design Patterns documentation** for submission: describe all **7 patterns** (Singleton, State, Strategy, Factory, Observer, Abstract Factory, Decorator) with class examples and UML references. Frame the `PlayableCharacter` → `Serin` → 3 forms system as "multiple playable characters" per rubric criteria, showing how adding a new character requires only one new `PlayableCharacter` subclass.
  - [ ] **Export class diagram** from architecture.puml as PNG/PDF for submission.
  - [ ] Weekly Report.

* **Developer B (Save/Load, Balance, QA & Demo)**
  - [ ] Complete `SaveLoadManager` to cleanly write/load all `RunState` fields. Verify that loading correctly restores all game state so the run resumes identically.
  - [ ] Implement "Load Game" flow from `MainMenuState` — detect save file, restore game state, resume at correct chamber.
  - [ ] Conduct balance pass: enemy HP/damage tuning, Momentum gain rates, Echo Power economy, boss phase difficulty curve.
  - [ ] Memory leak cleanup: verify all `unique_ptr` ownership is correct, no dangling references.
  - [ ] Conduct debugging, playtesting, and edge case handling (form switch during boss phase transition, save during mid-chamber, etc.).
  - [ ] End-of-game screen rendering for `GameOverState` (death/failure screen triggers chamber restart, distinct from the ending screens).
  - [ ] **Record demo video** (3–5 min) showing: main menu → Level 1–3 gameplay → form switching → Echo collection → boss fight → ending screen.

* **AI Agent Tasks**
  - [ ] End-of-game screen rendering boilerplate for `GameOverState`.
  - [ ] Save data serialization/deserialization test cases.
  - [ ] Automated balance spreadsheet: DPS calculations, time-to-kill per enemy per form, Momentum accrual rates.
  - **Prompt (Dev A provides):** *"Generate an enhanced GameOverState class. It receives EndingType from enums.hpp and renders the appropriate ending screen: ENDING_A_SHATTER (triumphant golden palette, bright effects), ENDING_B_RETREAT (somber blue-grey palette), ENDING_C_WARNING (dark red-black palette). Include: narrative text overlay with fade-in animation, a table showing all 5 Echoes with their final Power% and Collected/Stolen status, a 'Return to Main Menu' button. Use our UI framework (VerticalBox, Text, Button) and ColorPaletteManager."*

* **Deliverable**: A fully complete and polished game. Players can play from Level 1, progress through all levels with mid-chambers, save/load (with full `RunState` restored), defeat Malachar through all 4 phases, and view Ending A, B, or C. All 3 endings verified by manipulating `RunState.echoesStolen` (0, 1, 4). Confirm: all 4 boss phases transition at correct HP thresholds; Resonance Core bursts fire at 75%, 50%, 25% HP (doubled if fully intact); platforms collapse in Phase 3 and shrink in Phase 4; Hollow Bell reflect ward is active only in Phase 1 and expires at the transition; Marrow regen persists through Phases 2–4 but Obsidian blink stops at Phase 4. Submission deliverables ready: source code, class diagram, design pattern documentation, demo video.

---

## Post-Week 7: Additional Features

These features can be developed after the core game is complete and polished:

1. **Difficulty Modes** — Leverage `SettingManager::difficulty` field (already in architecture): Easy (enemy HP −20%, collection time −20%), Normal (default), Hard (enemy HP +30%, collection time +30%, Momentum gain −20%).
2. **Run Statistics Screen** — Post-game summary: total time, damage dealt/taken per form, fragments collected, Echo Power breakdown, enemies killed by type, form switch count.
3. **Speed Run Timer** — In-game timer overlay showing per-chamber and total completion time.
4. **Achievement System** — Unlock-based goals: "Collect all Echoes fully intact", "Complete a Gauntlet without switching forms", "Kill 3 enemies with one Riftcrush", etc.
5. **Bestiary / Codex** — In-game encyclopedia unlocked progressively: enemy descriptions, stats, weaknesses. Accessible from the pause menu.
6. **New Game+ Mode** — Replay with all Echoes' stolen-state modifiers active from the start, increased enemy count, boss starts at Phase 2 difficulty.
7. **Controller Support** — Gamepad input mapping via `SettingManager::keyBindings` extended to support controller axes and buttons.
8. **Screen Shake & Juice Effects** — Camera shake on heavy hits, flash on Echo damage, slow-motion on Special Ability activation, boss phase transition cinematics.
9. **Minimap** — Small corner minimap showing chamber layout, enemy positions (blips), Echo position, and exit gate location.
10. **Localization Framework** — String table system for UI text, supporting English and Vietnamese.
11. **Level Editor** *(Rubric bonus)* — Simple tool to create/edit chamber layouts and save as JSON files. Leverage existing `MapLoader` infrastructure.
12. **Second Playable Character + Character Select** *(Rubric: Multiple Players, 5 pts)* — Implement `CharacterSelectState` (already in architecture.puml) with a selection screen before gameplay. Create a second `PlayableCharacter` subclass with 3 unique forms (different stats, abilities, and animations). The `Player`/`PlayableCharacter` abstract factory architecture is already in place from Week 3 — adding a character requires only the new subclass + form implementations, no changes to `Player`, `Chamber`, `HUD`, or any other system.
