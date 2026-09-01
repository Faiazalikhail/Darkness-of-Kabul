# Code Review Findings

Measured on the current source. Each item records what was found, why it
matters, and whether it has been addressed.

## Measurements

| Metric | Before | After |
|---|---|---|
| Source files | 26 | 22 |
| Total source lines | 4,725 | 4,670 |
| Health implementations | 2 | 1 |
| Empty or unused classes | 3 | 0 |
| Settings that survive a relaunch | 1 of 4 | 4 of 4 |

The line total moves little because dead code was removed while a shared
component and a migration guide were added. The structural change is that one
concept is no longer implemented twice.

## Addressed

### Health was implemented twice

`APlayerCharacter` and `AZombieCharacter` each carried a maximum, a current
value, clamping, and a depletion test. Two copies of one rule drift apart: a
change to how damage clamps had to be made in both places or the two actors
would behave differently.

Both now own a `UHealthComponent`. The component holds the arithmetic; each
actor still decides what depletion means, because a ragdoll needs the impact
bone and direction while a game over needs none of that. The existing
`MaxHealth` and `CurrentHealth` properties were kept so Blueprint-authored
values and inspector readouts continue to work.

### Three classes existed but did nothing

`StonePickup`, `KabulHUDWidget`, and `TraversalCourse` were referenced by no
source file and no asset. `TraversalCourse` carried an 86-line constructor for
a feature no longer in the project. All three removed.

`SlingshotComponent` was deliberately kept: it is empty, but `BP_SlingShot`
references it, and it is the intended destination for the slingshot logic.

### Three of four settings reset on every launch

Master volume used `SetTransientPrimaryVolume`, which by definition does not
persist. Look sensitivity and field of view lived only in widget members. Only
display mode was saved. All four now load on startup and save when changed.

### Editor-generated files entering the repository

A `VisualStudioTools` plugin (50 files including PDFs and CI configuration) and
a regenerated `copilot-instructions.md` were being picked up by broad staging.
The project file had also been rewritten to enable that plugin and to drop the
`Editor`-only restriction on `ModelingToolsEditorMode`, which would have let it
load in Shipping. The project file was restored and all three paths are now
ignored.

## Outstanding

### The player class carries too many responsibilities

`APlayerCharacter` is 1,130 lines across 44 functions and owns movement, camera
shake, slingshot state, trajectory queries, health, settings, UI ownership, and
world reset. Its two longest functions are `TryClimb` at 132 lines and
`SetupPlayerInputComponent` at 127.

`SlingshotComponent.h` already states the intended fix in its own comment:
keeping the mechanic separate "lets the main mechanic evolve without turning the
player class into one large file."

**Not attempted deliberately.** Moving `StoneProjectileClass`, the charge times,
and the launch speeds onto a component would orphan the values currently
serialized in `BP_PlayerCharacter`. Silently losing the projectile class would
stop the game firing at all. This needs the editor open to re-enter the values
immediately after the move.

### UI layout is built in C++

`KabulPrototypeUI.cpp` is 1,079 lines, roughly 700 of them widget construction.
Unreal projects normally author widgets as UMG assets and keep C++ for
behaviour. See `UI_MIGRATION.md` for the naming contract and a per-screen plan.

**Not attempted deliberately.** Widget assets are binary and can only be created
in the editor. Deleting the C++ builders before the assets exist would leave the
project with no interface at all.

## Suggested order

1. Slingshot extraction, with the editor open to re-enter Blueprint values.
2. UI migration, one screen at a time, starting with the main menu.
