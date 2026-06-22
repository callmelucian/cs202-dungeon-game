# Echoes of the Ashen Vault — Project Implementation Plan & OOP Checklist

This document contains the 6-week detailed project plan and OOP architecture design for the 2-member C++ development team of **Echoes of the Ashen Vault**. 

A complete class diagram modeling these dependencies is available in [architecture.puml](architecture.puml).

---

## 1. OOP Architecture & Design Patterns Overview

### Core Object-Oriented Principles
* **Abstraction**: High-level abstract base classes (`Character`, `PlayerForm`, `Chamber`, `Item`, `StatusEffect`) declare interface contracts (`update()`, `draw()`, `takeDamage()`) without exposing concrete implementations.
* **Inheritance**: Models system relationships. `Player` and `Enemy` inherit from `Character`. Specific chamber types (`ProtectChamber`, `PreventChamber`, `GauntletChamber`, `MidChamber`, `BossChamber`) all inherit from `Chamber`.
* **Polymorphism**: Enables runtime dynamic dispatch. A single `std::vector<std::unique_ptr<Enemy>>` processes all active enemies polymorphically, and `PlayerForm*` swaps dynamically to alter player behaviors.
* **Encapsulation**: Keeps attributes `private` or `protected` and provides access control through setters/getters (e.g. `Character::takeDamage()`, `Player::gainMomentum()`).

### Integrated Design Patterns
1. **Singleton Pattern**: Applies to `Game`, `AssetManager`, `SoundManager`, `SaveLoadManager`, and `SettingManager` to guarantee single instances controlling critical engine services.
2. **State Pattern**: The stack-based `StateManager` manages active `GameState` implementations (`MainMenuState`, `GameplayState`, `PauseState`, `GameOverState`).
3. **Strategy Pattern**: The player controls a dynamic `PlayerForm` reference, swapping between `WraithbladeForm`, `VoidcasterForm`, and `IronshellForm` at runtime. Because `special1Threshold` is mutable at runtime (Hollow Bell Echo reduces it from 50 → 42.5 → 35), `PlayerForm` exposes a `setSpecial1Threshold()` setter that `RunState` calls whenever an Echo outcome is resolved.
4. **Factory Pattern**: `EnemyFactory` and `ChamberFactory` dynamically instantiate concrete classes from `EnemyType` enums and `ChamberConfig` data objects loaded by `MapLoader`.
5. **Observer Pattern**: `Echo` acts as a subject notifying registered `EchoObserver` instances of power percentage updates. `HUD` is the primary concrete observer, implementing `EchoObserver` to keep the on-screen Echo Power bar in sync without `Echo` needing any knowledge of rendering.

### Additional Architecture Notes
* **`RunState`** is a plain serialisable struct (no game-loop logic) that is the single source of truth for all persistent run data: current level/chamber, all five `EchoOutcome` values, HP carry-over, per-form Momentum snapshots, the Clarity Shard timer-reduction multiplier, the Hollow Bell `special1Threshold`, and the Foretell flag. Every system that needs to read or modify these values takes a `RunState&` parameter. `SaveLoadManager` serialises and deserialises it directly.
* **`CollisionSolver`** is a stateless utility class of static methods shared by `Player`, `Enemy`, and `Chamber` subsystems. It covers AABB, circle, and line-vs-rect/circle checks (the last required by Voidcaster's Special 1 wall-piercing shot).
* **`MapLoader`** is a stateless static class that reads structured external files and returns typed `ChamberConfig` and `WaveConfig` objects consumed by `ChamberFactory`. It is distinct from `SaveLoadManager`, which handles only player-progress serialisation.

---

## 2. 6-Week Granular Checklist & Task Breakdown

### Week 1: Core Engine, States & Asset Management (22.06 -> 27.06)

* **Developer A (Core Engine & States)**
  - [ ] Configure CMakeLists.txt to fetch and link dependencies (SFML/Raylib, etc.).
  - [ ] Implement `Game` singleton wrapper (`getInstance()`, `run()`, game loop, window setup).
  - [ ] Implement `SettingManager` singleton storing/loading game volumes, resolution, and control binds.
  - [ ] Implement stack-based `StateManager` (`pushState()`, `popState()`, `changeState()`).
  - [ ] Implement abstract `GameState` class interface.
  - [ ] Create concrete states: `MainMenuState` (Start, Load, Exit buttons), `GameplayState` (black window placeholder), `PauseState` (overlay), `GameOverState` (receives `EndingType` enum to display Ending A, B, or C).
* **Developer B (Resource & System Managers)**
  - [ ] Implement `AssetManager` singleton caching textures, fonts, and shaders.
  - [ ] Implement `SoundManager` singleton controlling background music streams and sound buffers.
  - [ ] Implement `SaveLoadManager` singleton with `saveGame(RunState&)` and `loadGame() → RunState` interfaces; keep it decoupled from map data (that is `MapLoader`'s job).
  - [ ] Define the `RunState` struct (fields: `currentLevel`, `currentChamber`, `map<EchoType, EchoOutcome> echoOutcomes`, `playerHP`, `activeForm`, per-form Momentum floats, `collectTimeReduction`, `special1MomentumThreshold`, `foretellActive`, `foretellPhase1`). This struct is used by every subsequent system and must exist before Week 2 work begins.
  - [ ] Integrate default UI font and basic game sound assets (menu navigation sounds, placeholder music).
* **AI-Assisted Tasks (Boilerplate & Utilities)**
  - [ ] Boilerplate CMake file configuration.
  - [ ] Standard C++ singleton template class structure.
  - [ ] Window initialization and rendering loop boilerplate.
  - [ ] `EndingType`, `EchoType`, `EchoOutcome`, `FormType`, `ChamberType`, and `EnemyType` enum definitions (one shared header; all other classes include it).
* **Week 1 Deliverable**: A compilable game window that boots into a working Main Menu. Clicking "Start" transitions to the gameplay screen; pressing Escape opens/closes the pause state; clicking "Exit" performs a clean shutdown.

---

### Week 2: Entity System & Player Forms
* **Developer A (Player & Strategy Forms)**
  - [ ] Implement abstract `Character` base class (position, velocity, HP/maxHP, moveSpeed, baseDamage, attackRange, attackRate, sprite, `vector<unique_ptr<StatusEffect>> activeEffects`, `takeDamage()`, `tickStatusEffects()`, `getEffectiveMoveSpeed()`).
  - [ ] Implement `Player` class inheriting from `Character`. The player owns all three `PlayerForm` instances at all times (`unique_ptr` members) and holds a raw `PlayerForm*` pointing at whichever is active. Also track `switchCooldownTimer`, `inMidChamber` flag (suspends the 4.0s cooldown when true), and the shared HP pool sized to the active form's `maxHP`.
  - [ ] Implement abstract `PlayerForm` Strategy base class with `attack()`, `useSpecial1()`, `useSpecial2()`, `onMomentumGainHit()`, `onMomentumGainDamageTaken()`, `addMomentum()`, `resetMomentum()`, and `setSpecial1Threshold()` (called at runtime by `RunState` when Hollow Bell is collected).
  - [ ] Code `WraithbladeForm` (Speed 7.0, MaxHP 100, Dmg 12, Range 1.5 melee, Rate 2/s; Momentum +6 per hit landed, +0.4 per Wraithblade-equivalent HP lost).
  - [ ] Code `VoidcasterForm` (Speed 5.0, MaxHP 70, Dmg 22, Range 12 piercing, Rate 1/s; Momentum +8 per far-range hit, +4 per additional pierced enemy, +0.4 per HP lost).
  - [ ] Code `IronshellForm` (Speed 2.5, MaxHP 160, Dmg 6, Range 1.0 cleave, Rate 1/s; Momentum +1.2 per Ironshell-equivalent HP lost, +3 per hit landed). Include `applySlowAura(vector<Enemy*>&)` as a per-tick method that applies/refreshes `SlowedEffect` on every enemy within 4.0 units — this is called by `Player::update()` whenever Ironshell is active.
* **Developer B (Movement, Input & Collisions)**
  - [ ] Bind WASD/D-pad to player velocity.
  - [ ] Implement `CollisionSolver` static utility class: `aabbIntersect()`, `circleIntersect()`, `resolveAABB()`, `lineIntersectsRect()`, and `lineIntersectsCircle()` (the last two are needed for Voidcaster's wall-piercing Special 1 in Week 5).
  - [ ] Implement the switch key binds (1, 2, 3) to swap active `PlayerForm` strategies. On switch: reset the outgoing form's Momentum to 0, carry the incoming form's frozen Momentum, apply the HP conversion formula `new_HP = (current_HP / old_max) × new_max`, and start the 4.0s `switchCooldownTimer`. Block switching if `switchCooldownTimer > 0` and `inMidChamber` is false.
  - [ ] Implement the HP conversion percentage formula: `new_HP = (current_HP / old_max) * new_max` on switch.
  - [ ] Implement 4.0s switch cooldown timer that blocks swapping but does not inhibit movement or combat.
* **AI-Assisted Tasks (Boilerplate & Utilities)**
  - [ ] Standard vector math utility functions (normalization, dot product, distance checks).
  - [ ] Static class attributes and getters/setters for `Character` and `PlayerForm`.
  - [ ] 2D AABB intersection checking code.
* **Week 2 Deliverable**: A controllable player character who can navigate a test grid, collide with walls, and switch between 3 forms using hotkeys. The HUD displays the active form and shows health converting dynamically based on maximum HP contributions. Verify the 4.0s swap cooldown is enforced and that switching while `inMidChamber = true` is unrestricted.

---

### Week 3: Status Effects & Basic Enemies
* **Developer A (Status Effects & Combat Hitboxes)**
  - [ ] Implement `StatusEffect` abstract base class with `onTick()`, `isExpired()`, `refresh()` (resets duration on reapplication rather than stacking), `dealerBaseDamage` (snapshot at application time so Burned ticks always use the original dealer's stat), and `getType()`.
  - [ ] Implement `BurnedEffect`: ticks once per second for 10 seconds, dealing `0.25 × dealerBaseDamage` per tick.
  - [ ] Implement `ParalyzedEffect`: 10-second duration (or 1.5 seconds when applied by Aegis Pulse); exposes `rollMiss()` returning true 40% of the time, re-rolled per action attempt.
  - [ ] Implement `SlowedEffect`: sets effective move speed to `base_speed × 0.70`; supports a `permanent` flag for zone-based slow (Drowned Archive water field) that ignores the 10-second expiry rule.
  - [ ] Wire `Character::tickStatusEffects(float dt)` into `Character::update()` to call `onTick()` on all active effects and remove expired ones.
  - [ ] Implement player melee and ranged attack hitboxes that scale by active form parameters.
* **Developer B (Enemy Base & Basic AI)**
  - [ ] Implement `Enemy` base class inheriting from `Character`. Add `isCarrier`, `isDecoy`, `fragmentDropCount`, `isRealCarrier()` virtual method (decoys override to return false), and `onDeath()` abstract method.
  - [ ] Implement `EnemyFactory::create(EnemyType, Vector2f)` mapping enums to concrete constructions.
  - [ ] Code simple steering AI behaviors: **Seek** (chases player position) and **Evade** (moves away from threat) as shared methods on `Enemy`.
  - [ ] Implement `WaterloggedScribe` (HP 18, Dmg 5, Speed 1.5 — effective after chamber water-slow; no additional Slowed multiplier unless additionally afflicted).
  - [ ] Implement `ShardSoldier` (HP 16, Dmg 6). Include a `selfHealActive` flag and `applySelfHeal(float dt)` method (heals 3% MaxHP/sec while not taking damage) — the flag is toggled on by `RunState` when Marrow Echo is stolen; defaults to false.
* **AI-Assisted Tasks (Boilerplate & Utilities)**
  - [ ] Timer-based status effect tick boilerplate.
  - [ ] Standard steering behavior algorithms (Seek, Flee, Obstacle Avoidance).
  - [ ] Class constructors for basic enemies via the factory.
* **Week 3 Deliverable**: A playable scene containing basic spawned enemies that chase the player. The player can attack, deal damage, and apply status effects (visible via floating labels or status icons above the entities). Verify `SlowedEffect` permanent flag works correctly (zone slow persists beyond 10s).

---

### Week 4: Chamber Logic & Fragment Economy
* **Developer A (Chamber System & Protect/Prevent Modes)**
  - [ ] Implement abstract `Chamber` base class with `onEnter(Player&, RunState&)`, `update()`, `render()`, `isComplete()`, `spawnEnemies()`, and `handleCollisions()`.
  - [ ] Implement `ChamberFactory::create(ChamberConfig&)` and wire it to `MapLoader`.
  - [ ] Implement `MapLoader` static class: `loadChamber(string path) → ChamberConfig` and `loadWaves(string path) → vector<WaveConfig>`. Define `ChamberConfig` (chamberName, type, associatedEcho, requiredCollectionTime, waves, wall rects, echoPosition, playerSpawn, exitPosition, carrierCount, decoyCount) and `WaveConfig` (enemyGroups, spawnDelay, spawnPositions) data structs.
  - [ ] Code `ProtectChamber`: collection radius 2.5 units, collection timer (counts up while Serin is in radius, pauses on exit, does not reset), `onEchoHit()` applying −8% Echo Power (floor 10%), `onFragmentCollected(bool midCollection)` granting +5% pre-collection or +2.5% mid-collection, `checkIronshellRedirect(Player&)` redirecting 100% of incoming Echo damage to Serin when she is within 1.0 unit of the Echo, and `applyWraithbladeKnockback(Enemy*)` pushing enemies 4 units away from Echo position.
  - [ ] Code `PreventChamber`: tracks `associatedEcho` field so it can write `RunState.echoOutcomes[associatedEcho] = STOLEN` if any real carrier reaches the exit. `onCarrierHit(Enemy*, bool lethal)` triggers the 0.5s stagger animation on real carriers (non-lethal hit only); decoys show no reaction.
* **Developer B (Echo Observer, Fragment Economy & Save/Load)**
  - [ ] Implement `Item` abstract base class. Implement `EchoFragment` inheriting from `Item`: physics position/velocity, `update()`, `draw()`, `checkCollection(Player&)`, and `onPickup(Player&)`.
  - [ ] Implement `Echo` subject class: `power` (float, clamped 10–100), `addFragmentPower(bool midCollection)`, `takeDamage()` (−8%), `markCollected()`, `markStolen()`, `isFullyIntact()` (power ≥ 90), observer list with `addObserver()` / `removeObserver()` / `notifyObservers()`.
  - [ ] Implement `EchoObserver` abstract base. Implement `HUD` as its concrete observer: `onEchoPowerChanged()`, `onEchoCollected()`, `onEchoStolen()` keep the HUD's Echo Power bar in sync. Register `HUD` with `Echo` in `GameplayState::onEnter()`.
  - [ ] Implement fragment multiplier logic by active form: Wraithblade knockback-wall collision → +1 extra fragment (add `checkKnockbackWallBonus(Enemy*, Chamber*)` to `WraithbladeForm`); Voidcaster pierce-kill chain → +1 per additional enemy beyond the first killed in one shot (add `checkCarrierReveal(vector<Enemy*>&)` to `VoidcasterForm`); Ironshell kill while enemy is Slowed → fragment drop doubled (handled in `Enemy::onDeath()` by checking active effects).
  - [ ] Implement Prevent Chamber fragment bonus: carrier kill → 3 fragments instead of 1 (set `fragmentDropCount = 3` on carrier enemies in `EnemyFactory`).
  - [ ] Wire `SaveLoadManager::saveGame(RunState&)` and `loadGame() → RunState` to correctly serialise all `RunState` fields established in Week 1.
* **AI-Assisted Tasks (Boilerplate & Utilities)**
  - [ ] JSON/CSV map reading parser functions for `MapLoader`.
  - [ ] Standard implementation of the Observer pattern for `Echo` and `HUD`.
  - [ ] Basic properties for fragment spawning physics.
* **Week 4 Deliverable**: A fully playable Level 1 Chamber 1 (Protect) and Chamber 2 (Prevent). Players can collect fragments pre- and mid-collection, defend the Echo, trigger form-specific fragment bonuses, and trigger state saving/loading. Verify that mid-collection fragments grant only +2.5% (not +5%) Echo Power and that Ironshell's Damage Redirect correctly subtracts from Serin's HP rather than the Echo's Power.

---

### Week 5: Advanced Chambers, AI & Boss Setup
* **Developer A (Gauntlet, Mid-Chamber & Boss Phase 1)**
  - [ ] Implement `GauntletChamber`: wave tracker, 0s gap between waves (next wave begins the instant the last enemy of the prior wave dies — no delay), and `applyGauntletHeal(Player&)` granting +25% Max HP heal on chamber clear (applied to whichever form is active at exit, using the HP conversion formula if the player switches afterward). This heal triggers at the end of Level 1 Ch.3, Level 2 Ch.3, and Level 3 Ch.3.
  - [ ] Implement `MidChamber`: sets `Player::inMidChamber = true` on `onEnter()` (suspending the 4.0s switch cooldown entirely) and `false` on `onPlayerExit()`. On exit, grant the last-active form a flat +15 Momentum bonus (clamped to 100).
  - [ ] Create `BossChamber` wrapping `BossMalachar`. `BossChamber` owns the platform layout (6 platforms, radius 3 each for Phase 3; Phase 4 shrinks each at 0.1 units/sec, floor 1.5 units). Phase and platform state are managed here; AI behaviour is delegated to `BossMalachar`.
  - [ ] Create `BossMalachar` class. Implement Phase 1 behaviour: **Void Bolt Cycle** (3 ranged bolts, Dmg 14 each, fired 0.8s apart, 0.5s charge-glow telegraph — extended to 1.1s total if Foretell is active from a fully-intact Clarity Shard); **Summoning Burst** (spawns 2× Shard Wraith, HP 30, Dmg 8, fly toward Serin). Fixed 12-second repeating cycle. Phase 1→2 transition check at 75% HP (937.5 HP).
  - [ ] On phase transition, read `RunState` to apply all relevant modifiers: Hollow Bell stolen → activate `reflectWardActive` (first hit every 8s reflects 20% back to Serin, Phase 1 only); Foretell from Clarity Shard collected at any Power → begin extended telegraphs in Phase 2 (or Phase 1 if fully intact).
* **Developer B (Advanced Enemy Mechanics)**
  - [ ] Implement `SiegeWraith` (HP 40, Dmg 10; on death calls `explode()` dealing 15 AOE damage in a 3-unit radius — can chain into other nearby enemies including other Siege Wraiths).
  - [ ] Implement `ChoirHusk` (HP 20 standard / 16 carrier / 22 guard, Dmg 7, Speed 4.0). Call-and-response: one Husk "calls," triggering `triggerCallResponse()` which gives up to 2 nearby Husks a 0.6s shared windup before all strike together. Bonus: killing 2+ Husks within the same 0.6s window grants +1 bonus fragment (tracked in chamber update).
  - [ ] Implement `ResonantCantor` (HP 50, Dmg 9, does not move). Every 5 seconds emits a pulse applying `SlowedEffect` to Serin if within 6 units. Slow refreshes on re-application rather than stacking in magnitude.
  - [ ] Implement `HushedStalker` (HP 22, Dmg 9). Invisible until 0.4s attack wind-up telegraph. `onSlowedApplied()` sets `visible = true` for the Slowed effect's duration. Kills while Slowed drop 2 fragments and do not trigger the +1-spawn noise penalty for that specific kill. The noise mechanic (`+1 spawn per Serin offensive action`, hard cap +12 total) is tracked in `ProtectChamber` (Level 3, Chamber 1) and fed to `EnemyFactory`.
  - [ ] Implement `MirrorBearer` (HP 18 carrier / 24 guard, Speed 6.5). 1 real carrier, 2 decoys (randomised per attempt). Real carrier: `isRealCarrier()` returns true, staggers visibly for 0.5s on any non-lethal hit, drops 3 fragments on death. Decoy: no knockback reaction, no stagger, `isRealCarrier()` returns false, 0 fragments, shatters into smoke on death.
  - [ ] Implement `VoidShunter` (HP 25, Dmg 11 charge-only, Speed 6.0 when charging). Wraithblade knockback (4 units) and Riftcrush can push Shunters into the Hunger Pit void (instant kill). A Shunter's charge that connects while Serin is within 2 units of the pit edge pushes her 3 units — check `CollisionSolver` for pit boundary.
  - [ ] Implement `SarcophagusWarden` (HP 28, Dmg 10, Speed 4.5). Every 6s one Warden enters Guard Stance for 2s: immune to knockback, +50% damage resistance. Voidcaster pierce shots bypass the resistance (check `FormType` in `takeDamage()` override).
* **AI-Assisted Tasks (Boilerplate & Utilities)**
  - [ ] Wave spawner timers and count checks.
  - [ ] Invisibility shader/sprite blending code for `HushedStalker`.
  - [ ] Exploding entity radius checker algorithm (reuse `CollisionSolver::circleIntersect()`).
* **Week 5 Deliverable**: Complete playable Level 2 and Level 3 chambers featuring advanced enemy interactions (invisible stalkers, mirror decoys, exploding Siege Wraiths). The player completes Level 3 Chamber 4 "Sarcophagus Approach" (decoy reliquary: successful 10s collect grants +20% Max HP buff for the Final Chamber only) and enters the boss arena. Verify: `MidChamber` suspends cooldown and grants +15 Momentum on exit; `GauntletChamber` wave gap is truly 0s; Stalker spawns are capped at +12.

---

### Week 6: Boss Phases, Serialization & Polish
* **Developer A (Boss Phases 2, 3, 4 & Endings)**
  - [ ] Implement Phase 2 (HP ≤ 75% → HP ≤ 50%): read `RunState` to conditionally enable Marrow Echo regen (`applyMarrowRegen(float dt)`, 2% MaxHP/sec = 25 HP/sec, active through Phases 2–4) and Obsidian Key blink (`performBlink()`, every 6–9s randomised, active through Phases 2–3). Trigger Resonance Core transition burst on the 75% threshold crossing: `resonanceCoreBurst()` deals 8% of Malachar's current HP (doubled to two sequential bursts ~1s apart if Resonance Core was fully intact).
  - [ ] Implement Phase 3 (HP ≤ 50% → HP ≤ 25%): notify `BossChamber` to shatter the floor into 6 floating platforms. Add **Platform Sunder** attack every 15 seconds: telegraphs Serin's current platform, which collapses 3 seconds after telegraph (forcing relocation). Carry over Phase 2 modifiers (Marrow regen, Obsidian blink). Trigger second Resonance Core transition burst at the 50% threshold crossing.
  - [ ] Implement Phase 4 (HP ≤ 25% → HP = 0): notify `BossChamber` to begin shrinking platforms at 0.1 units/sec (floor radius 1.5). Add **Soul Lance** attack every 10 seconds: single targeted bolt, Dmg 30, 1.0s telegraph (extended to 1.6s if Foretell active). Carry over Marrow regen if active; Obsidian Key blink does **not** carry into Phase 4 (shrinking platforms replace it). Trigger third Resonance Core transition burst at the 25% threshold crossing.
  - [ ] Implement ending checker in `GameplayState` after boss defeat: count `RunState.echoesStolen`. 0 stolen → `EndingType::ENDING_A_SHATTER`; 1–2 stolen → `EndingType::ENDING_B_RETREAT`; 3–5 stolen → `EndingType::ENDING_C_WARNING`. Push `GameOverState(ending)`.
* **Developer B (HUD, Audio, Save/Load & QA)**
  - [ ] Complete `HUD` as concrete `EchoObserver`: render health bar (sized to active form's MaxHP), active status effect icons, active form indicator, 3 persistent Momentum meters (one per form, frozen meters visually distinct), switch cooldown indicator, and Echo Power bar.
  - [ ] Complete `SaveLoadManager` to cleanly write/load all `RunState` fields. Verify that loading correctly restores per-form Momentum, Echo outcomes, `special1MomentumThreshold`, and Foretell flags so the run resumes identically.
  - [ ] Integrate sound effects (form switching, sword swings, projectile fire, enemy hit, fragment pickup, Echo collection, boss phase transitions).
  - [ ] Conduct debugging, playtesting, balance adjustments, and memory leak cleanup.
* **AI-Assisted Tasks (Boilerplate & Utilities)**
  - [ ] HUD layout positioning coordinates.
  - [ ] Save data serialization structures for `RunState`.
  - [ ] End-of-game screen rendering boilerplate for `GameOverState`.
* **Week 6 Deliverable**: A fully complete and polished game. Players can play from Level 1, progress through all levels, save/load (with full `RunState` restored), defeat Malachar through all 4 phases, and view Ending A, B, or C. Verify: Resonance Core bursts fire at 75%, 50%, and 25% HP thresholds; platforms collapse in Phase 3 and shrink in Phase 4; Hollow Bell's reflect ward expires at the Phase 1 → Phase 2 transition regardless of cooldown state; Marrow regen persists through Phases 2, 3, and 4 but Obsidian blink stops at Phase 4.

---

## 3. Weekly Deliverable Verification Checklist

To confirm project health, at the end of each week, the development team must verify the following:

* [ ] **Week 1 Deliverable**: Compile and run the project. Boot into the main menu, select "Start Game" to switch to gameplay, press Escape to pause, and click "Quit" to ensure it returns 0. Confirm `RunState` struct compiles and all shared enums (`EchoType`, `FormType`, `EchoOutcome`, `EndingType`, `ChamberType`, `EnemyType`) are accessible across translation units.
* [ ] **Week 2 Deliverable**: Move player. Switch forms (1, 2, 3). Check that HP converts correctly (e.g. 50/100 Wraithblade becomes 80/160 Ironshell) and that the 4-second swap cooldown is enforced. Verify that switching sets the outgoing form's Momentum to 0 and resumes the incoming form's frozen Momentum value. Temporarily set `inMidChamber = true` and confirm the cooldown is suspended.
* [ ] **Week 3 Deliverable**: Spawn basic enemies. Attack them to verify hitboxes register, damage is subtracted, and status icons (Burned/Slowed/Paralyzed) display correctly above target. Apply `SlowedEffect` with `permanent = true` and confirm it does not expire after 10 seconds.
* [ ] **Week 4 Deliverable**: Play Chamber 1 (Protect) and Chamber 2 (Prevent). Check that pre-collection fragments grant +5% Echo Power and mid-collection fragments grant +2.5%. Confirm Ironshell's Damage Redirect subtracts from Serin's HP (not Echo Power) when within 1 unit. Confirm hitting real Bone Sprinter carriers triggers the stagger animation; decoys show no reaction. Save and reload; verify `RunState` is restored identically.
* [ ] **Week 5 Deliverable**: Play Gauntlet waves, verify 0s delay transition. Confirm `MidChamber` suspends the switch cooldown and grants +15 Momentum to the exit form. Test all advanced enemy actions: Hushed Stalkers turn visible on `SlowedEffect` application; their kills while Slowed drop 2 fragments without triggering the noise-spawn penalty; Siege Wraith death AOE chains into adjacent enemies; MirrorBearer decoys show no knockback or stagger; VoidShunter can be knocked into the Hunger Pit void; SarcophagusWarden Guard Stance blocks normal damage but not Voidcaster pierce shots.
* [ ] **Week 6 Deliverable**: Fight Malachar. Confirm Phase transitions occur at exactly 75%, 50%, and 25% HP. Confirm Resonance Core burst fires at each transition (doubled if fully intact). Confirm platforms collapse in Phase 3 and shrink in Phase 4. Confirm Hollow Bell reflect ward is active only in Phase 1 and expires at the transition. Confirm Marrow regen persists through Phases 2–4. Confirm Obsidian blink stops at Phase 4. Trigger all three endings by manipulating `RunState.echoesStolen` (0, 1, 4) and verify `GameOverState` displays the correct ending screen.
