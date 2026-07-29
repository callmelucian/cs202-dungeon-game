# Weekly Report Content

## General Information

- Group ID: 53
- Group name: Echoes of the Ashen Vault.
- Project name: Echoes of the Ashen Vault (Dungeon Game).
- Date range: 27.07.2026 - 01.08.2026

## Tasks completed this week

### 25125027 - Nguyễn Tấn Minh:

- Re-design the base `Chamber` class to following the Single Responsibility Principle.
- Implement `GauntletChamber`: wave tracker, 0s gap between waves (next wave begins the instant the last enemy of the prior wave dies — no delay), `waves : vector<vector<EnemyType>>`, `currentWaveIndex`, and `applyGauntletHeal(Player&)` granting +25% Max HP heal on chamber clear. This heal triggers at the end of Level 1 Ch.3, Level 2 Ch.3, and Level 3 Ch.3.
- Implement `MidChamber`: sets `Player::inMidChamber = true` on `onEnter()` (suspending the 4.0s switch cooldown entirely) and `false` on `onPlayerExit()`. On exit, grant the last-active form a flat +15 Momentum bonus (clamped to 100).
- Reimplement chamber grid logic (support more complicated objects like stairs, etc.).
- Implement level progression flow in `GameplayState: Level 1 -> Level 2 -> Level 3 -> Boss`. Wire chamber completion → next chamber transition with Mid-Chambers in between.
- Implement chamber retry on death (restart current chamber, preserve prior chamber results per §6.2).
- Create Level 2 and Level 3 chamber layout data files (JSON/CSV) for MapLoader.
- Reimplement logic camera view.
- Create `BossChamber` wrapping `BossMalachar`. `BossChamber` owns the platform layout (6 platforms, radius 3 each for Phase 3; Phase 4 shrinks each at 0.1 units/sec, floor 1.5 units). Phase and platform state are managed here; AI behaviour is delegated to `BossMalachar`.
- Implement `BossMalachar` class with `currentPhase`, `transitionPhase(int)`, `platformSunder(Chamber&)`, `soulLance(Chamber&)` (per architecture.puml).
- Implement Phase 1 behaviour: **Void Bolt Cycle** (3 ranged bolts, Dmg 14 each, fired 0.8s apart, 0.5s charge-glow telegraph — extended to 1.1s total if Foretell is active from a fully-intact Clarity Shard); **Summoning Burst** (spawns 2× Shard Wraith, HP 30, Dmg 8, fly toward Serin). Fixed 12-second repeating cycle. Phase 1→2 transition check at 75% HP (937.5 HP).
- Implement `ShardWraith` enemy (HP 30, Dmg 8) — spawned by BossMalachar Phase 1.
- Implement Phase 2 (HP ≤ 75% → HP ≤ 50%): read `RunState` to conditionally enable Marrow Echo regen (`applyMarrowRegen(float dt)`, 2% MaxHP/sec = 25 HP/sec, active through Phases 2–4) and Obsidian Key blink (`performBlink()`, every 6–9s randomised, active through Phases 2–3). Trigger Resonance Core transition burst on the 75% threshold crossing: `resonanceCoreBurst()` deals 8% of Malachar's current HP (doubled to two sequential bursts ~1s apart if Resonance Core was fully intact).
- Implement Phase 3 (HP ≤ 50% → HP ≤ 25%): notify `BossChamber` to shatter the floor into 6 floating platforms. Add **Platform Sunder** attack every 15 seconds: telegraphs Serin's current platform, which collapses 3 seconds after telegraph (forcing relocation). Carry over Phase 2 modifiers (Marrow regen, Obsidian blink). Trigger second Resonance Core transition burst at the 50% threshold crossing.
- Implement Phase 4 (HP ≤ 25% → HP = 0): notify `BossChamber` to begin shrinking platforms at 0.1 units/sec (floor radius 1.5). Add **Soul Lance** attack every 10 seconds: single targeted bolt, Dmg 30, 1.0s telegraph (extended to 1.6s if Foretell active). Carry over Marrow regen if active; Obsidian Key blink does **not** carry into Phase 4 (shrinking platforms replace it). Trigger third Resonance Core transition burst at the 25% threshold crossing.
- On phase transition, read `RunState` to apply all relevant modifiers: Hollow Bell stolen → activate `reflectWardActive` (first hit every 8s reflects 20% back to Serin, Phase 1 only); Foretell from Clarity Shard collected at any Power → begin extended telegraphs in Phase 2 (or Phase 1 if fully intact).
- Boss sprite/animation assets — design and implement multi-phase boss visuals.

### 25125061 - Châu Tấn Phát:

- Implement `SiegeWraith` (HP 40, Dmg 10; on death calls `explode()` dealing 15 AOE damage in a 3-unit radius — can chain into other nearby enemies including other Siege Wraiths).
- Implement `ChoirHusk` (HP 20 standard / 16 carrier / 22 guard, Dmg 7, Speed 4.0). Call-and-response: one Husk "calls," triggering `triggerCallResponse()` which gives up to 2 nearby Husks a 0.6s shared windup before all strike together. Bonus: killing 2+ Husks within the same 0.6s window grants +1 bonus fragment (tracked in chamber update).
- Implement `ResonantCantor` (HP 50, Dmg 9, does not move). Every 5 seconds emits a pulse applying `SlowedEffect` to Serin if within 6 units. Slow refreshes on re-application rather than stacking in magnitude.
- Implement `HushedStalker` (HP 22, Dmg 9). Invisible until 0.4s attack wind-up telegraph. `onSlowedApplied()` sets `visible = true` for the Slowed effect's duration. Kills while Slowed drop 2 fragments and do not trigger the +1-spawn noise penalty for that specific kill. The noise mechanic (`+1 spawn per Serin offensive action`, hard cap +12 total) is tracked in `ProtectChamber` (Level 3, Chamber 1) and fed to `EnemyFactory`.
- Implement `MirrorBearer` (HP 18 carrier / 24 guard, Speed 6.5). 1 real carrier, 2 decoys (randomised per attempt). Real carrier: `isRealCarrier()` returns true, staggers visibly for 0.5s on any non-lethal hit, drops 3 fragments on death. Decoy: no knockback reaction, no stagger, `isRealCarrier()` returns false, 0 fragments, shatters into smoke on death.
- Implement `VoidShunter` (HP 25, Dmg 11 charge-only, Speed 6.0 when charging).
- Implement basic camera system: follow player position, smooth lerp, clamp to chamber bounds.
- Complete `HUD` as concrete `EchoObserver`: render health bar (sized to active form's MaxHP), active status effect icons, active form indicator, 3 persistent Momentum meters (one per form, frozen meters visually distinct), switch cooldown indicator, and Echo Power bar via `onEchoPowerChanged()`.
- Integrate sound effects (form switching, sword swings, projectile fire, enemy hit, fragment pickup, Echo collection, boss phase transitions).
- Design and implement particle effects: fragment pickup sparkle, Echo collection glow, enemy death poof, boss phase transition bursts.
- Wire `SaveLoadManager` to correctly serialise all `RunState` fields. Verify that loading correctly restores per-form Momentum, Echo outcomes, `special1MomentumThreshold`, and Foretell flags so the run resumes identically.

## AI Usage Declaration

Tasks done by AI agents using Gemini model, via Antigravity:
  - Wave spawner timers and count checks.
  - Invisibility shader/sprite blending code for `HushedStalker`.
  - Exploding entity radius checker algorithm (reuse `CollisionSolver::circleIntersect()`).
  - Enemy sprite generation prompts for new enemy types.
  - HUD layout positioning coordinates and bar rendering.
  - Save data serialization structures for `RunState`.
  - Boss attack pattern timer/state-machine boilerplate.
  - Particle system basic framework.

## Issues

No issues encountered during this week.