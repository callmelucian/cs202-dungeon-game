# Echoes of the Ashen Vault — Game Design Document

---

## World & Premise

The **Ashen Vault** is a colossal underground fortress built by a forgotten civilization. Buried beneath it is **Malachar**, a lich-king who survived his own death by binding his soul to stolen power fragments called **Echoes**. He has been slowly awakening, sending his thralls to collect more Echoes to fuel his resurrection.

You play as **Serin**, a relic hunter who accidentally triggered the Vault's seal. Now trapped inside, your only way out is through Malachar himself.

---

## Win Condition

Clear both levels and defeat Malachar in the Vault's heart. The ending you receive depends on how many Echoes you collected, how intact they are, and how many were stolen by enemies.

---

## Player Forms

Serin has three forms, switchable at any time during a run. The tradeoffs for switching are a **cooldown timer between consecutive switches** and **momentum reset**.

| Form | Name | Speed | HP | Damage | Identity |
|---|---|---|---|---|---|
| Close attack | **Wraithblade** | High | Medium | Low | Spectral short-sword. Every strike knocks enemies backward through a rift. Fast and relentless. |
| Far attack | **Voidcaster** | Medium | Low | High | Channels condensed void energy into long-range bolts that pierce through multiple enemies. |
| Defensive | **Ironshell** | Very low | Very high | Very low | Petrified bark-armor. Absorbs enormous punishment. Passive aura slows nearby enemies. |

### Form switching

- Serin can switch forms at any point during a run — inside chambers, mid-combat, even during collection timers.
- After switching, a **switch cooldown** must expire before switching again. The cooldown is the same regardless of which forms are involved.
- There is no penalty to stats, momentum, or Echo integrity from switching. The cooldown is the only cost.
- This means a player can legitimately clear threats as one form, switch to finish a collection, and switch again to handle the next wave — as long as they respect the cooldown window.

---

## Systems

### Resource Economy — Echo Fragments

Echoes are not simply collected or lost. Their final power is built up through active decisions during the chamber.

- **Echo Fragments** drop from enemies killed inside Protect and Prevent chambers.
- Every fragment collected before the Echo is picked up adds to that Echo's total power.
- Killing more enemies yields a stronger Echo — but killing takes time, during which the Echo remains exposed.
- The player chooses their own balance: **fight aggressively for a more powerful Echo** vs. **rush the collection for a safer, weaker one**.

This makes aggression vs. caution a real strategic axis on every Protect and Prevent chamber, rather than a passive skill check.

**Fragment yield by form:**

| Form | Fragment behaviour |
|---|---|
| Wraithblade | Knocked-back enemies that hit a wall drop an extra fragment on death. |
| Voidcaster | Pierce-kills (hitting multiple enemies with one shot) yield a fragment bonus per additional target hit. |
| Ironshell | Enemies that die while slowed by the aura drop double fragments. |

Each form has a natural way to farm fragments, encouraging players to lean into their form's strengths rather than playing generically.

---

### Momentum System

Each form has its own Momentum meter that fills differently and unlocks a distinct ability at high charge. Momentum rewards commitment to a form's identity and gives players a reason to stay in a form rather than switching reactively.

Each player has 2 levels of special ability, and the player can choose when to use the special ability depending on the amount of momentum that form has collected. Using the special ability or form switching resets the momentum.

| Form | Momentum builds on | Special ability 1 | Special ability 2|
|---|---|---|---|
| Wraithblade | Hit (major) & Damage taken (minor) | The next knockback strike deals more damage and sends the enemy careening into all nearby enemies, hitting each of them. | In the next 10 seconds, Wraithblade's sword contains fire that burns all enemies hit by it. |
| Voidcaster | Far-range hit (major) & Damage taken (minor) | A single charged shot that pierces walls and hits every enemy in a straight line across the chamber. | In the next 10 seconds, Voidcaster's shot causes an explosion after hitting the enemies. |
| Ironshell | Damage taken (major) & hit (minor) | A brief shockwave pulse that radiates outward from Serin, staggering all nearby enemies and knocking fragments out of them early | In the next 10 seconds, Ironshell has an aura veil that paralyzes and knocks fragments out of them early the enemies whenever they touch it.

---

### Effects

Effects can be applied to both enemies (dealt by Serin) and to Serin (dealt by Malachar). A character can be applied multiple effects. An effect takes place in 10 seconds and apply special effects on the applied characters as follows:

| Name | Effect |
|---|---|
| Burned | Reduce HP every seconds by an amount depending on the ATK stats of the dealer |
| Paralyzed | There is a 40% chance that the character misses during paralysis |
| Slowed | Reduce the speed of the character by 30% |

---

### Mid-Chambers

During mid-chambers, a small chamber in between main chambers, players can freely switch form and the last form to exit the mid-chambers will gain an additional momentum on top of its existing momentum.

---

## Chamber Types

Each chamber falls into one of four categories. Every category supports all three forms — there is no correct form for any chamber, only different approaches.

### Protect

An Echo is present in the chamber and is threatened by enemies. The goal is to collect it while minimising the damage enemies deal to it.

- Collection requires Serin to stay near the Echo for a set amount of time.
- Each hit the Echo takes during collection removes a percentage of its final power.
- Enemies killed before collection begins drop fragments; enemies killed during collection drop fewer.
- The player decides when to start collecting vs. how many enemies to clear first.

**Form approaches:**

- Wraithblade: knock enemies away continuously while the collection timer runs.
- Voidcaster: clear the room from range before collection starts, then collect safely.
- Ironshell: stand on the Echo's location and absorb all incoming damage through the shell instead of letting it reach the Echo.

### Prevent

Enemies are attempting to carry an Echo to an exit and deliver it to Malachar. The goal is to stop every carrier before they escape.

- Multiple enemies carry Echoes (real or decoy). The real carrier must be eliminated.
- Fragment drops from carriers are higher than from regular enemies — aggressive interception is rewarded.

**Form approaches:**

- Wraithblade: close in fast and kill carriers before they build speed.
- Voidcaster: pierce-shot carriers from across the chamber, potentially hitting multiple in one bolt.
- Ironshell: physically block the exit corridor — the shell is wide enough to body-block the path entirely, forcing carriers to turn back.

### Gauntlet

A wave-clearing chamber with no Echo present. Its purpose is to drain Serin's resources before the boss. HP carries over to the next chamber.

- Multiple waves of enemies spawn without pause between them.
- No collection objective. The only goal is to survive and exit.
- Fragment drops do not apply here — this chamber is pure attrition management.

**Form approaches:**

- Wraithblade: kill quickly, thin the herd aggressively, keep enemies off-balance with constant knockback.
- Voidcaster: prioritise high-threat targets; pierce-shots let you hit multiple enemies per bolt.
- Ironshell: absorb the waves passively, let enemies exhaust themselves, kill stragglers. Builds Momentum rapidly through absorbed damage.

### Final Chamber

The boss fight against Malachar. His power is shaped by every Echo stolen across the run, and Serin's power is shaped by every Echo collected.

See the **Final Boss** section below.

---

## Level Structure

### Level 1 — The Outer Wards

Malachar's outer defenses, partially abandoned but newly reactivated when Serin triggered the seal. Three chambers.

---

**Chamber 1 · The Drowned Archive** — *Protect*

> *"The water hasn't killed the ink — it's still moving. So are they."*

A flooded library. Water slows movement for enemies and Serin alike. The Echo sits on a half-submerged reading table at the centre of the room.

- **Echo:** Clarity Shard — reduces the collection time for all future Echoes. Fully intact: also reveals Malachar's attack patterns one phase early.
- **Enemies:** Waterlogged Scribes (slow, attack in tight groups) — 6 total.
- **Collection time:** 8 seconds of proximity.
- **Fragment opportunity:** scribes move slowly in the water, making them easy targets for Ironshell's aura or Voidcaster's pierce.

---

**Chamber 2 · The Bone Corridor** — *Prevent*

> *"They're not just running — they know exactly where they're going."*

A long, narrow hallway lined with skeletal remains mounted to the walls. Fast thralls sprint toward a far exit gate the moment the chamber begins.

- **Echo:** Marrow Echo — if stolen, Malachar gains HP regeneration and the next Gauntlet spawns self-healing enemies.
- **Enemies:** Bone Sprinters (very fast, low HP) — 4 carriers, 3 blockers.
- **Mechanic:** Blockers position themselves between Serin and the carriers. Real carriers have a faint luminous flicker.

---

**Chamber 3 · The Collapsed Barracks** — *Gauntlet*

> *"Three waves. I counted. Malachar wants me tired, not dead. Not yet."*

A ruined soldier's quarters. Rubble creates choke points. Three waves attack without pause.

- **Enemies:** Wave 1: 5 Shard Soldiers. Wave 2: 4 Soldiers + 1 Siege Wraith. Wave 3: 3 Soldiers + 2 Siege Wraiths.
- **Mechanic:** Siege Wraiths deal AOE damage in a burst when destroyed. Can be baited into each other's death zones.

---

### Level 2 — The Inner Sanctum

Malachar's active domain. The air hums with stolen energy. Four chambers, each more punishing than the last.

---

**Chamber 1 · The Resonance Hall** — *Protect*

> *"Every sound I make wakes another one. So I'll have to be still. For now."*

A cathedral-like space where sound echoes violently. The Echo rests at the altar. Every attack Serin makes draws reinforcements — sound is a resource here.

- **Echo:** Resonance Core — deals burst damage to Malachar during phase transitions. Fully intact: the burst triggers twice per transition.
- **Enemies:** Hushed Stalkers (invisible until they attack) — 8 base, plus one additional Stalker per attack Serin makes.
- **Collection time:** 12 seconds of proximity.
- **Fragment opportunity:** Stalkers that Ironshell's aura slows become visible. Killing slowed Stalkers drops double fragments without triggering the noise mechanic.

---

**Chamber 2 · The Mirror Vault** — *Prevent*

> *"One of them is lying. Possibly all of them. This is Malachar's sense of humor."*

A room of reflective obsidian walls. Three carriers each hold what appears to be the Echo. Only one is real.

- **Echo:** Obsidian Key — if stolen, Malachar gains a phase-shift blink in Phase 2 and the next chamber's exits begin flickering.
- **Enemies:** Mirror Bearers — 3 carriers (1 real, 2 decoys), 2 guards.
- **Mechanic:** Decoys shatter into smoke on death. The real carrier staggers. Voidcaster pierce-shots reveal this difference at range. Wraithblade's knockback also reveals it — decoys don't react to being pushed.

---

**Chamber 3 · The Hunger Pit** — *Gauntlet*

> *"He built a room whose entire purpose is to throw me into nothing. Thoughtful."*

A circular arena with a void at the centre. Enemies try to push Serin in. Five waves, each spawning closer to Serin's starting position.

- **Enemies:** Void Shunters (medium HP, charge attacks) — 4 per wave, 20 total.
- **Mechanic:** Falling into the pit is instant death. Wraithblade's knockback can push enemies into the pit — but their charge attacks can push Serin in too.

---

**The Vault's Heart — Final Chamber** — *Boss*

> *"So this is what I woke up."*

An ancient throne room slowly crumbling into the void. Malachar rises from a crystal sarcophagus.

---

## Final Boss — Malachar

Malachar's fight is shaped entirely by what happened across all previous chambers.

### Phase 1 — Baseline

Malachar opens with his base moveset regardless of the run's history: cycling between ranged void bolts and a summoning burst that spawns 2 Shard Wraiths. No modifiers active yet.

### Phase 2 — Echo Absorption

Malachar absorbs every Echo stolen by his thralls. Each stolen Echo activates a modifier:

- **Marrow Echo stolen:** Malachar regenerates HP continuously throughout Phase 2 and 3. Must be out-damaged.
- **Obsidian Key stolen:** Malachar blinks unpredictably between attacks, making targeting difficult.
- **Both stolen:** both modifiers apply simultaneously.

### Phase 3 — Vault Collapse

The floor shatters into floating platforms. Malachar becomes airborne.

- **Resonance Core collected:** platforms glow during Malachar's phase transitions and deal burst damage to him. Fully intact Core triggers this twice per transition.
- **Clarity Shard collected:** Serin can read Malachar's next attack one beat early (a brief visual tell). Fully intact: this starts in Phase 1 instead.
- **Neither collected:** Phase 3 runs at full difficulty with no player bonuses.

### Endings

**Ending A — The Shatter** *(all Echoes collected, none stolen)*

Serin collects the Resonance Core's final pulse and drives it into Malachar's soul-crystal. He dissolves, asking only: *"How?"* The Vault shudders open. Serin walks out into sunlight.

**Ending B — The Retreat** *(some Echoes stolen)*

Malachar is defeated but not destroyed. He retreats into the deep Vault, his voice fading: *"Next time, hunter."* The exit opens — barely. Serin leaves knowing the job is unfinished.

**Ending C — The Warning** *(most Echoes stolen)*

A brutal, exhausting fight ends with Malachar dissolving — but laughing. *"You barely survived. I'll be stronger next time."* Serin exits alone. A sequel hook pulses in the silence.

---

## Design Summary

| System | Purpose |
|---|---|
| Free form switching (cooldown only) | Maximises player expression — no build-locking, no penalties beyond timing |
| Echo Fragment economy | Makes aggression vs. caution a live decision on every Protect and Prevent chamber |
| Momentum (per form, persistent) | Rewards leaning into a form's identity; each form has a distinct high-reward ability |
| Three form approaches per chamber | No chamber has a correct answer — every form can win through a different route |
