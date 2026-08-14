# Echoes of the Ashen Vault — Week 08 Task Allocation Plan (Zero-Dependency Split)

**Document Overview:** This document distributes all remaining outstanding tasks, feature alignments, and expansion items between **Developer A** and **Developer B** with **100% Zero Cross-Developer Dependencies**. Each developer owns distinct, decoupled sub-systems and file directories to guarantee zero merge conflicts and completely independent development workflows.

---

## Zero-Dependency Architecture & Boundary Split

```mermaid
graph TD
    subgraph Dev A: Player, Forms, Combat & Co-op
        direction TB
        A1[Task A1: Wraithblade Special 2 Cinderveil]
        A2[Task A2: Voidcaster Special 1 & 2 Lance/Detonation]
        A3[Task A3: Ironshell Special 1 & 2 Aegis/Veil]
        A4[Task A4: Form Base Move Speeds & Stats]
        A5[Task A5: Form Momentum Gain Formulas]
        A6[Task A6: Hollow Bell Threshold Sync in Player]
        A7[Task A7: Combat Fragment Bonuses & Drop Tables]
        A8[Task A8: Marrow Gauntlet Self-Heal Link]
        A9[Task A9: Multiple Players / Local Co-op Engine]
    end

    subgraph Dev B: World, Chambers, UI/UX, Hazards & Expansion
        direction TB
        B1[Task B1: Clarity Shard Protect Timer Reduction]
        B2[Task B2: Run Ending Determination Logic]
        B3[Task B3: Campaign Map Mid-Chamber Flow]
        B4[Task B4: L3Ch1 Resonance Hall Noise Mechanic]
        B5[Task B5: L3Ch3 Hunger Pit Instant Death Void]
        B6[Task B6: L3Ch4 Reliquary Buff & Obsidian Opacity]
        B7[Task B7: Per-Chamber Timer Overlay in HUD]
        B8[Task B8: Minimap Radar Widget in HUD]
        B9[Task B9: Inter-Chamber Run Statistics Screen]
        B10[Task B10: Difficulty Modes Selection & Scaling]
    end
```

### Module Ownership & File Directory Isolation

| Domain | Developer A (Player & Combat) | Developer B (World, UI & Systems) |
| :--- | :--- | :--- |
| **Directory Ownership** | `src/entities/player*`, `src/entities/forms/`, `src/entities/effects/`, `src/economy/` | `src/chambers/`, `src/ui/`, `src/core/states/`, `assets/maps/` |
| **Cross-Dependency** | **0% (Zero)** | **0% (Zero)** |
| **Merge Conflict Risk** | **0% (Zero)** | **0% (Zero)** |

---

## Developer A — Player Entities, Forms, Combat & Multiplayer Co-op

*Directory Ownership:* `src/entities/player*`, `src/entities/forms/`, `src/entities/effects/`, `src/economy/`

### Task A1: Implement Wraithblade Special 2 (Cinderveil)
- [x] **Target File:** [`src/entities/forms/wraithblade-form.cpp`](file:///d:/Code/CS202%20Lab/cs202-dungeon-game/src/entities/forms/wraithblade-form.cpp)
- [x] **Priority:** P2 — High
- [x] **Description:** Replace placeholder attack speed buff with Cinderveil mode. For 10 seconds, every hit landed by Wraithblade applies `BurnedEffect` (dealing $0.25 \times \text{base damage} = 3.0$ dmg/sec for 10 seconds).

### Task A2: Implement Voidcaster Special 1 & 2 (Lance of the Hollow & Detonation Field)
- [x] **Target Files:** [`src/entities/forms/voidcaster-form.cpp`](file:///d:/Code/CS202%20Lab/cs202-dungeon-game/src/entities/forms/voidcaster-form.cpp), [`src/utils/collision-solver.cpp`](file:///d:/Code/CS202%20Lab/cs202-dungeon-game/src/utils/collision-solver.cpp)
- [x] **Priority:** P2 — High
- [x] **Description:** 
  - [x] **Special 1 (Lance of the Hollow):** Modify raycast collision checks to allow Lance to **pierce walls and obstacles** across the chamber for 2.5x base damage.
  - [x] **Special 2 (Detonation Field):** Replace movement speed placeholder with 10s buff where every Voidcaster projectile landing triggers a 2.5-unit radius explosion dealing 0.75x base damage.

### Task A3: Implement Ironshell Special 1 & 2 (Aegis Pulse & Veil of Thorns)
- [x] **Target File:** [`src/entities/forms/ironshell-form.cpp`](file:///d:/Code/CS202%20Lab/cs202-dungeon-game/src/entities/forms/ironshell-form.cpp)
- [x] **Priority:** P2 — High
- [x] **Description:**
  - [x] **Special 1 (Aegis Pulse):** 5.0-unit radius shockwave that staggers nearby enemies (`ParalyzedEffect(1.5f)`) and knocks out 1 Echo Fragment from each enemy hit.
  - [x] **Special 2 (Veil of Thorns):** Replace +50 defense placeholder with 10s 4.0-unit aura that applies `ParalyzedEffect` to touching enemies and knocks out 1 fragment immediately.

### Task A4: Align Player Form Movement Speeds & Base Stats
- [x] **Target Files:** `src/entities/forms/wraithblade-form.cpp`, `voidcaster-form.cpp`, `ironshell-form.cpp`
- [x] **Priority:** P3 — Medium
- [x] **Description:** Adjust base movement speeds to match spec values (§14.1):
  - [x] Wraithblade: **7.0 units/s** (currently 9.0)
  - [x] Voidcaster: **5.0 units/s** (currently 6.5)
  - [x] Ironshell: **2.5 units/s** (currently 3.25)

### Task A5: Align Momentum Gain Formulas
- [x] **Target Files:** `src/entities/forms/wraithblade-form.cpp`, `voidcaster-form.cpp`
- [x] **Priority:** P3 — Medium
- [x] **Description:**
  - [x] Wraithblade: Update hit gain to **+6 Momentum / hit** (currently +5).
  - [x] Voidcaster: Add distance-check requirement for +8 Momentum gain (+4 bonus per extra enemy pierced).

### Task A6: Sync Hollow Bell Momentum Threshold in Player
- [x] **Target File:** [`src/entities/player.cpp`](file:///d:/Code/CS202%20Lab/cs202-dungeon-game/src/entities/player.cpp)
- [x] **Priority:** P2 — High
- [x] **Description:** Implement internal momentum threshold reader in `Player` so Special 1 threshold dynamically unlocks at 42.5 or 35 Momentum when Hollow Bell is collected/intact.

### Task A7: Combat Fragment Bonuses & Guard Drop Tables
- [x] **Target Files:** [`src/entities/enemy/bone-sprinter.cpp`](file:///d:/Code/CS202%20Lab/cs202-dungeon-game/src/entities/enemy/bone-sprinter.cpp), `choir-husk.cpp`, `mirror-bearer.cpp`
- [x] **Priority:** P3 — Medium
- [x] **Description:**
  - [x] Set `fragmentDropCount = 1` for non-carrying blocker/guard variants.
  - [x] Implement Wraithblade wall knockback kill bonus (+1 extra fragment).
  - [x] Implement Choir Husk dual-kill bonus (+1 extra fragment when killing 2+ Husks within 0.6s windup).

### Task A8: Link Marrow Echo Gauntlet Enemy Self-Heal
- [x] **Target File:** [`src/entities/enemy/shard-soldier.cpp`](file:///d:/Code/CS202%20Lab/cs202-dungeon-game/src/entities/enemy/shard-soldier.cpp)
- [x] **Priority:** P3 — Medium
- [x] **Description:** Check `runState.echoOutcomes[EchoType::MARROW] == EchoOutcome::STOLEN` before enabling 3% Max HP/s self-heal on non-Siege enemies in L1Ch3.

### Task A9: Multiple Players / Local Co-op Support
- [ ] **Target Files:** [`src/entities/player.hpp`](file:///d:/Code/CS202%20Lab/cs202-dungeon-game/src/entities/player.hpp), [`src/entities/player.cpp`](file:///d:/Code/CS202%20Lab/cs202-dungeon-game/src/entities/player.cpp)
- [ ] **Priority:** EXPANSION
- [ ] **Description:** Refactor `Player` class and input mapping to support multiple player instances (Player 1 on Keyboard/Mouse, Player 2 on Gamepad) for local co-op play.

---

## Developer B — World, Chambers, UI/UX, Hazards & Expansion Features

*Directory Ownership:* `src/chambers/`, `src/ui/`, `src/core/states/`, `assets/maps/`

### Task B1: Apply Clarity Shard Protect Timer Reduction
- [ ] **Target File:** [`src/chambers/chamber-factory.cpp`](file:///d:/Code/CS202%20Lab/cs202-dungeon-game/src/chambers/chamber-factory.cpp)
- [ ] **Priority:** P2 — High
- [ ] **Description:** Multiply `requiredCollectionTime` in `ProtectChamber` by `runState.collectTimeReduction` (10% or 20% timer reduction when Clarity Shard is collected/intact).

### Task B2: Implement Run Ending Determination Logic
- [ ] **Target File:** [`src/core/states/game-play-state.cpp`](file:///d:/Code/CS202%20Lab/cs202-dungeon-game/src/core/states/game-play-state.cpp)
- [ ] **Priority:** P2 — High
- [ ] **Description:** Replace hardcoded `ENDING_A_SHATTER` with dynamic calculation based on total stolen Echoes:
  - [ ] 0 Stolen $\rightarrow$ Ending A (The Shatter)
  - [ ] 1–2 Stolen $\rightarrow$ Ending B (The Retreat)
  - [ ] 3–5 Stolen $\rightarrow$ Ending C (The Warning)

### Task B3: Fix Campaign Map Mid-Chamber Flow
- [ ] **Target File:** [`assets/maps/campaign.json`](file:///d:/Code/CS202%20Lab/cs202-dungeon-game/assets/maps/campaign.json)
- [ ] **Priority:** P2 — High
- [ ] **Description:** Re-order `campaign.json` so Mid-Chambers occur *between* main chambers (e.g. L1Ch1 $\rightarrow$ Mid $\rightarrow$ L1Ch2 $\rightarrow$ Mid $\rightarrow$ L1Ch3).

### Task B4: Implement L3Ch1 Resonance Hall Noise Mechanic
- [ ] **Target File:** [`src/chambers/protect-chamber.cpp`](file:///d:/Code/CS202%20Lab/cs202-dungeon-game/src/chambers/protect-chamber.cpp)
- [ ] **Priority:** P3 — Medium
- [ ] **Description:** Increment noise counter on player attack in L3Ch1, spawning +1 Hushed Stalker per attack (capped at +12). Killing a Slowed Stalker drops 2 fragments silently without triggering noise spawn.

### Task B5: Implement L3Ch3 Hunger Pit Instant Death Void
- [ ] **Target File:** [`src/chambers/boss-chamber.cpp`](file:///d:/Code/CS202%20Lab/cs202-dungeon-game/src/chambers/boss-chamber.cpp) or `gauntlet-chamber.cpp`
- [ ] **Priority:** P2 — High
- [ ] **Description:** Add 5-unit radius central void hazard circle causing instant death on contact for both player and knockback-pushed enemies.

### Task B6: L3Ch4 Sarcophagus Reliquary Buff & Obsidian Pit Edge Opacity
- [ ] **Target Files:** `protect-chamber.cpp`, `gauntlet-chamber.cpp`
- [ ] **Priority:** P4 — Low
- [ ] **Description:**
  - [ ] **L3Ch4:** Successfully defending decoy reliquary grants a one-time +20% Max HP buff for the final boss fight.
  - [ ] **Obsidian Key:** Reduce pit edge warning glow opacity by 60% in Hunger Pit if Obsidian Key was stolen.

### Task B7: Implement Per-Chamber Timer Overlay in HUD
- [ ] **Target File:** [`src/ui/widgets/hud.cpp`](file:///d:/Code/CS202%20Lab/cs202-dungeon-game/src/ui/widgets/hud.cpp)
- [ ] **Priority:** EXPANSION
- [ ] **Description:** Add a clean digital timer widget to the HUD displaying elapsed chamber time.

### Task B8: Implement Minimap Radar Widget in HUD
- [ ] **Target File:** `src/ui/widgets/minimap.cpp` [NEW]
- [ ] **Priority:** EXPANSION
- [ ] **Description:** Create corner radar UI widget rendering miniature tiles for chamber walls, player dot (blue), exit gate (red), and enemy dots (yellow).

### Task B9: Implement Inter-Chamber Run Statistics Screen
- [ ] **Target File:** `src/core/states/inter-chamber-state.cpp` [NEW]
- [ ] **Priority:** EXPANSION
- [ ] **Description:** Create UI transition state between chambers displaying player stat boosts, active enemy/boss modifiers from stolen Echoes, banked fragments, and total time elapsed.

### Task B10: Implement Difficulty Modes Selection & Scaling
- [ ] **Target Files:** `src/global-settings/setting-manager.hpp`, `src/chambers/chamber-factory.cpp`
- [ ] **Priority:** EXPANSION
- [ ] **Description:** Add `DifficultyMode` enum (Easy, Normal, Hard / Nightmare) and apply multiplier scaling to enemy HP, damage, and speed during chamber instantiation.

---

## Parallel Execution Timeline

Because there are **0 cross-developer dependencies**, both developers can execute their respective tasks completely in parallel from Day 1 to Day 5 without waiting for one another:

```mermaid
gantt
    title Parallel Zero-Dependency Sprint Schedule
    dateFormat  YYYY-MM-DD
    
    section Developer A (Player & Combat)
    Dev A: Special Abilities (A1, A2, A3)    :active, dev_a1, 2026-08-10, 2d
    Dev A: Stats & Momentum Tuning (A4-A6)   :dev_a2, after dev_a1, 1d
    Dev A: Drops & Self-Heal Links (A7, A8)  :dev_a3, after dev_a2, 1d
    Dev A: Local Co-op Engine (A9)           :dev_a4, after dev_a3, 1d
    
    section Developer B (World, UI & Systems)
    Dev B: Echo Modifiers & Endings (B1-B3)  :active, dev_b1, 2026-08-10, 1d
    Dev B: Hazards & Noise Mechanics (B4-B6) :dev_b2, after dev_b1, 2d
    Dev B: HUD Timer & Minimap (B7, B8)       :dev_b3, after dev_b2, 1d
    Dev B: Inter-Chamber Stats & Difficulty (B9, B10) :dev_b4, after dev_b3, 1d
```
