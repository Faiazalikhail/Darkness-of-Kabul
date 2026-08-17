# Zombie AI — Editor Setup

Follow in order. **Step 1 is mandatory** — without a navigation mesh the zombies will not move at all, no matter what the code does.

---

## Step 1 — Build the navigation mesh (required, ~2 minutes)

Navigation is what lets a zombie path toward you. The level has none yet.

1. Open `/Game/Maps/L_TargetPractice`.
2. In the top-left **Place Actors** panel (if hidden: **Window ▸ Place Actors**), type `Nav Mesh` in the search box.
3. Drag **Nav Mesh Bounds Volume** into the level.
4. Select it in the World Outliner. In the **Details** panel find **Transform ▸ Scale**.
5. Scale it until the orange wireframe box covers the whole playable area — the ground where you and the zombies walk. Start with scale `X=60, Y=60, Z=15` and adjust. It must enclose the floor *and* have headroom above it.
6. Also set **Location Z** so the box's bottom sits slightly *below* the floor.
7. Press **P** to toggle navigation display. **Correct result: the walkable floor turns green.**

> **If no green appears:** the volume is too small, sitting above the floor, or the floor geometry is set to *Not Navigable*. Enlarge the box and drop it lower.
>
> **If green appears only in patches:** that's fine — zombies path only on green. Extend the volume to cover any area you want them to reach.

**Verify:** press Play. Shoot one zombie. It should turn and walk at you. If it stays still, navigation is not built — return to this step before doing anything else.

---

## Step 2 — Confirm AI possession (~1 minute)

The C++ sets this already, but a Blueprint child can hold an older saved value that overrides it.

1. Open `Content/Blueprints/BP_ZombieCharacter`.
2. Click **Class Defaults** in the toolbar.
3. In **Details**, search `Auto Possess`.
4. Set **Auto Possess AI** = **Placed in World or Spawned**.
5. Search `AI Controller`. Set **AI Controller Class** = **AIController**.
6. **Compile**, then **Save**.

---

## Step 3 — Tune the AI (~1 minute, optional)

Same **Class Defaults** screen, category **Zombie | AI**. All values are live-editable without recompiling:

| Setting | Default | Raise it to… |
|---|---|---|
| `Sight Range` | 2600 | notice you from further away |
| `Sight Half Angle Degrees` | 75 | widen peripheral vision (180 = sees all around) |
| `Chase Max Walk Speed` | 520 | **make them faster / more intense** |
| `Attack Range` | 175 | let them swing from further |
| `Attack Damage` | 25 | kill in fewer hits (100 health = 4 hits) |
| `Attack Interval` | 1.1 | lower = swings land faster |
| `Disturbance Radius` | 1800 | **how far a hit spreads to the group** |
| `Sense Interval` | 0.2 | lower = reacts faster, costs more CPU |

**For maximum intensity:** `Chase Max Walk Speed` 620, `Attack Interval` 0.8, `Disturbance Radius` 2200.

**For the "hit one of four, three come" demo:** place four zombies within ~1500 units of each other, and a fifth well outside 1800 units. Hit one — three join, the distant one stays idle. That contrast is the thing worth showing.

---

## Step 4 — Wire the animations (~5 minutes)

The pack at `Content/ZombieAnimationPack/Animations/Mannequin_UE4/` already has what you need: `anim_Walk_A`, `anim_Run_A`, `anim_Idle_A`, `anim_Attack_A`–`D`.

### 4a — Movement (idle ⇄ walk/run)

If `BP_ZombieCharacter`'s mesh already has an Animation Blueprint that blends on speed, **it will just work** — the AI drives `Velocity`, and the existing graph reacts. Test first; only continue if they slide without animating.

If you need to build it:

1. Right-click in `Content/Blueprints` ▸ **Animation ▸ Animation Blueprint**.
2. Pick the zombie's skeleton. Name it `ABP_Zombie`.
3. In the **AnimGraph**, right-click ▸ add a **Blend Space** or **Blendspace Player**, or simplest: add a **State Machine** with two states — Idle (`anim_Idle_A`) and Walk (`anim_Walk_A` or `anim_Run_A`).
4. Transition Idle → Walk when `Speed > 10`; Walk → Idle when `Speed < 10`.
5. To get `Speed`: in the **Event Graph**, use **Event Blueprint Update Animation** ▸ **Try Get Pawn Owner** ▸ **Get Velocity** ▸ **VectorLength** ▸ promote to a variable named `Speed`.
6. Compile and Save.
7. Open `BP_ZombieCharacter`, select the **Mesh** component, set **Anim Class** = `ABP_Zombie`. Compile, Save.

### 4b — Attack swing

The C++ fires an event each time a swing lands. Hook the animation to it:

1. Open `BP_ZombieCharacter` ▸ **Event Graph**.
2. Right-click ▸ search **On Zombie Attack** ▸ add the event. (It carries a `Player Location` pin if you want to face the player.)
3. Drag from the execution pin ▸ **Play Anim Montage** (or **Play Montage** with the Mesh's anim instance).
4. Set the montage to an attack animation. *If `anim_Attack_A` is a plain Animation Sequence, right-click it in the Content Browser ▸ **Create ▸ Create AnimMontage** first.*
5. Compile and Save.

### 4c — Alert reaction (optional)

Same graph: add the **On Zombie Alerted** event and play a growl sound or a lunge animation. Fires once when a zombie wakes up.

---

## Step 5 — Test the full loop (~2 minutes)

1. **Play.** Walk into a zombie's view → it should notice and come at you.
2. **Hide behind a wall** → it loses line of sight and won't wake (before it's alerted).
3. **Shoot one zombie in a group of four** → that one plus its neighbours converge; a distant zombie stays put.
4. **Let one reach you** → health drops 25% per swing, HUD `HEALTH` counter turns red.
5. **Take four hits** → `YOU WERE OVERRUN` screen with **Try Again** and **Leave**.
6. **Press R** → health restored, zombies reset to unaware and return to their placed positions.
7. **Shoot a zombie in the leg while it chases** → it drops to crawling and keeps coming, slower.
8. **Shoot a chasing zombie in the torso** → it staggers for 2 seconds, stops, then resumes.

---

## If something goes wrong

| Symptom | Cause | Fix |
|---|---|---|
| Zombies never move | No navigation mesh | Step 1. Press **P** — no green means no navigation |
| Zombies move but never attack | `Attack Range` too small vs capsule size | Raise `Attack Range` to 220 |
| They notice you through walls | Level geometry not blocking `Visibility` | Check the wall's collision preset |
| They all wake at once | `Disturbance Radius` too large | Lower it, and spread the zombies further apart |
| They slide without animating | No Anim Class on the mesh | Step 4a |
| No attack animation | Event not hooked, or asset is a sequence not a montage | Step 4b |
| Player takes no damage | Zombie can't reach you (gap in navigation) | Check the green area is continuous |

---

## What the code does (for your presentation)

- **Sensing is timer-driven, not per-frame.** Each zombie senses every `0.2 s` with a randomised first offset, so 35 placed zombies don't all think on the same frame.
- **Detection needs three conditions:** within `Sight Range`, inside the forward cone (dot product against `cos(75°)`), and an unobstructed `ECC_Visibility` line trace. Distance is compared squared to avoid a square root.
- **The disturbance is radius-limited.** `RaiseDisturbance` iterates zombies and wakes only those within `DisturbanceRadius`, which is why hitting one pulls its group rather than the level.
- **Existing systems are untouched.** Stagger suspends chase and attack; crawling keeps its 80 speed; ragdoll death halts the AI; `R` clears every alert and restores health.
