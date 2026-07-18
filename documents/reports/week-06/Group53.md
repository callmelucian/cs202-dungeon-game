# Weekly Report Content

## General Information

- Group ID: 53
- Group name: Echoes of the Ashen Vault.
- Project name: Echoes of the Ashen Vault (Dungeon Game).
- Date range: 13.07.2026 - 18.07.2026

## Tasks completed this week

### 25125027 - Nguyễn Tấn Minh:

- Implement abstract `Chamber` base class with `player : Player&`, `enemies : vector<unique_ptr<Enemy>>`, `items : vector<unique_ptr<Item>>`, `isCompleted`, `update(dt)`, `draw(window)`, `spawnEnemy()`, `checkCollisions()` (per architecture.puml).
- Implement `ChamberFactory::createChamber(level, chamberIndex, Player&) → unique_ptr<Chamber>` (per architecture.puml).
- Implement `MapLoader` static class: `loadChamber(string path) → ChamberConfig` and `loadWaves(string path) → vector<WaveConfig>`. Define `ChamberConfig` and `WaveConfig` data structs.
- Implement basic tilemap renderer: load tile textures, render floor/wall grid from `ChamberConfig` wall rects, support water tiles for Drowned Archive.
- Code `ProtectChamber`: collection radius 2.5 units, collection timer, `onEchoHit()` applying −8% Echo Power, `onFragmentCollected(bool midCollection)` granting +5% / +2.5% Echo Power, `checkIronshellRedirect(Player&)` redirecting 100% of incoming Echo damage to Serin when she is within 1.0 unit of the Echo, and `applyWraithbladeKnockback(Enemy*)` pushing enemies 4 units away from Echo position.
- Code `PreventChamber`: tracks `associatedEcho` field so it can write `RunState.echoOutcomes[associatedEcho] = STOLEN` if any real carrier reaches the exit. `onCarrierHit(Enemy*, bool lethal)` triggers the 0.5s stagger animation on real carriers (non-lethal hit only); decoys show no reaction.
- *Task from week 2 (delayed because code dependency):* Include `applySlowAura(vector<Enemy*>&)` as a per-tick method that applies/refreshes `SlowedEffect` on every enemy within 4.0 units — this is called by `Player::update()` whenever Ironshell is active.

### 25125061 - Châu Tấn Phát:

- Implement `SpecialAbilityState` abstract base class, which delegates to `innerState` and holds a `StatModifier` and duration timer.
- Implement the 6 concrete Special Ability Decorator states:
  - Wraithblade: `WraithbladeRiftcrushState`, `WraithbladeCinderveilState`
  - Voidcaster: `VoidcasterLanceState`, `VoidcasterDetonationFieldState`
  - Ironshell: `IronshellAegisPulseState`, `IronshellVeilOfThornsState`
- Wire these states into `Player::triggerSpecial()`, which instantiates the decorator state and enters it in `PlayerCombatStateMachine`.
- Implement `Item` abstract base class with `position`, `bounds`, `onCollect(Player&, Chamber&)` (per architecture.puml Economy package).
- Implement `EchoFragment` inheriting from `Item`: physics position/velocity, `value`, `update()`, `draw()`, `onCollect()`.
- Implement `Echo` abstract Subject class: `name`, `echoPower` (float, clamped 10–100), `observers : vector<EchoObserver*>`, `takeDamage(penalty)`, `addPower(amount)`, `getPower()`, `attach(observer)`, `notify()` (per architecture.puml).
- Implement concrete Echo subclasses: `ClarityShard`, `MarrowEcho`, `HollowBell`, `ResonanceCore`, `ObsidianKey` (per architecture.puml — each stores its specific collected/stolen effects).
- Implement `EchoObserver` interface with `onEchoPowerChanged(float power)`. Implement `GameplayState` as a concrete observer.
- Implement fragment multiplier logic by active form: Wraithblade knockback-wall collision → +1 extra fragment; Voidcaster pierce-kill chain → +1 per additional enemy beyond the first killed in one shot; Ironshell kill while enemy is Slowed → fragment drop doubled.
- Implement Prevent Chamber fragment bonus: carrier kill → 3 fragments instead of 1.
- Create Level 1 chamber layout data files (JSON/CSV) for MapLoader.

## AI Usage Declaration

Tasks done by AI agents using Gemini model, via Antigravity:
  - JSON/CSV map reading parser functions for `MapLoader`.
  - Standard Observer pattern boilerplate for `Echo` → `EchoObserver`.
  - Basic fragment spawning physics (velocity scatter, gravity, collection magnet).
  - Tilemap rendering grid loop boilerplate.

## Issues

No issues encountered during this week.