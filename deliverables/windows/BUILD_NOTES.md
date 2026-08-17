# Windows Portfolio Build Notes

- **Build date:** 2026-08-17 (America/Toronto)
- **Unreal version:** 5.6.1
- **Project version:** 0.2.0 Prototype Beta
- **Target:** `DarknessOfKabul` Win64
- **Configuration:** Shipping
- **Startup map:** `/Game/Maps/L_TargetPractice`
- **Output path:** `deliverables/windows/DarknessOfKabul_Windows_Portfolio/`
- **Executable:** `DarknessOfKabul.exe`
- **Package size:** 0.74 GB
- **Compile:** Succeeded. Editor Development and Shipping targets both completed cleanly.
- **Blueprint compile:** Succeeded with 0 errors, 0 warnings, and 0 failed loads.
- **Cook:** Succeeded. IoStore processed 380 packages without imports.
- **Packaging:** Succeeded with Pak, IoStore compression, and Windows prerequisites.
- **Launch smoke test:** The packaged executable started and ran stably.

## Implemented flow

1. Short UMG loading presentation.
2. Press-any-button welcome screen.
3. Main menu with New Game, disabled Continue, Settings, and Leave.
4. Settings for volume, look sensitivity, field of view, and fullscreen/windowed display.
5. Gameplay HUD with title/build text, objective progress, health, reticle, charge state, and controls.
6. Escape pause menu with Resume, Play Again, Settings, Main Menu, and Leave.
7. Reset confirmation prompt on `R`.
8. Completion screen after every zombie is defeated and every wobble target is hit.
9. Game over screen when the zombies reduce the player to zero health.

## Slingshot verification scope

- The original 3.5-second overdraw branch cancels the pull and aim state and triggers the camera kick.
- The real stone and predictor both use a maximum of two collisions.
- Both use the stone radius, gravity, bounciness, friction, collision channel, and eight-second lifetime.
- Zombie impacts display a body marker; surfaces display only short incoming/outgoing angle pieces and the final marker.
- A zombie body hit stops the stone. A first wall impact may continue to a second and final impact.

## Enemy behaviour

- Zombies sense on a per-zombie timer using range, a forward vision cone, and a line-of-sight trace.
- Striking a zombie raises a disturbance that alerts only those within its radius, so a hit pulls the local group rather than the level. The disturbance is not blocked by walls.
- Alerted zombies chase, stop at attack range, and swing on an interval, applying damage to the player.
- An alerted zombie that ends up beyond `LoseInterestDistance` gives up and returns to unaware.
- Torso hits stagger a standing zombie. Leg hits force crawling, and a crawling zombie stays down when struck again.
- Head or lethal hits activate the ragdoll with an impulse along the stone's travel direction.

## Reset behaviour

`R` opens a confirmation prompt. Accepting restores the player start, player health, live stones, placed zombies, zombie alert state, and objective progress. Escape or Cancel returns to play without resetting.

## Remaining hands-on checks

1. Rehearse a preferred wall-to-enemy indirect camera angle.
2. Confirm the chase and attack pacing feels right after any tuning changes.
3. Confirm the attack animation reads clearly at the moment damage lands.
4. Complete every placed objective once to review the completion screen timing.

## Known limitations

- The loading presentation is a post-map-load UMG transition, not an asynchronous MoviePlayer screen.
- Continue is intentionally unavailable because the prototype has no save system.
- Hit zones depend on Physics Asset bone names; unknown bones fall back to the torso.
- Volume, look sensitivity, and field of view reset to defaults on relaunch. Only display mode persists.
- External Fab/Megascans, zombie-pack, and WILDPOLY licence records are incomplete. Do not publish the package until those records are verified.
