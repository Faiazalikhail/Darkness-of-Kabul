# Windows Portfolio Build Notes

- **Build date:** 2026-08-14 (America/Toronto)
- **Unreal version:** 5.6.1, changelist 44394996
- **Project version:** 0.2.0 Prototype Beta
- **Target:** `DarknessOfKabul` Win64
- **Configuration:** Shipping
- **Startup map:** `/Game/Maps/L_TargetPractice`
- **Output path:** `deliverables/windows/DarknessOfKabul_Windows_Portfolio/`
- **Executable:** `DarknessOfKabul.exe`
- **Compile:** Succeeded. Final Shipping and Development Editor targets completed cleanly.
- **Blueprint compile:** Succeeded with 0 errors, 0 warnings, and 0 failed loads.
- **Cook:** Succeeded for `L_TargetPractice` and its dependencies. IoStore processed 648 packages.
- **Packaging:** Succeeded with Pak, IoStore, and Windows prerequisites.
- **Packaged visual validation:** Welcome, main menu, disabled Continue, settings, New Game, gameplay HUD, reticle, objective counters, and the clean game/build identity were checked in the Win64 Shipping window.
- **Final startup smoke test:** The refreshed package opened and exited successfully in unattended benchmark mode.
- **Map objectives observed:** 35 zombies and 8 wobble targets.
- **Aim presentation observed:** The old continuous trajectory line is absent. UMG shows the center reticle and compact predicted-impact markers.
- **Archive validation:** Every compressed archive entry was read successfully without decompression errors.

## Implemented beta flow

1. Short UMG loading presentation.
2. Press-any-button welcome screen.
3. Main menu with New Game, disabled Continue, Settings, and Leave.
4. Settings for volume, look sensitivity, field of view, and fullscreen/windowed display.
5. Gameplay HUD with title/build text, objective progress, reticle, charge state, and controls.
6. Escape pause menu with Resume, Play Again, Settings, Main Menu, and Leave.
7. Completion screen after every placed zombie is defeated and every wobble target is hit, with Play Again and Leave.

## Slingshot verification scope

- The original 3.5-second overdraw branch is restored: it cancels the pull and aim state and triggers the camera kick.
- The real stone and predictor both use a maximum of two collisions.
- Both use the stone radius, gravity, bounciness, friction, collision channel, and eight-second lifetime.
- Zombie impacts display a body marker; surfaces display only short incoming/outgoing angle pieces and the final marker.
- A zombie body hit stops the stone. A first wall impact may continue to a second and final enemy or surface impact.
- `R` restores the player start, removes live stones, resets placed targets, and clears objective progress.

## Remaining hands-on checks

1. Rehearse a preferred wall-to-enemy indirect camera angle in the packaged map.
2. Hold a pull past 3.5 seconds and judge the restored camera kick by feel.
3. Confirm Escape pause/resume while actively holding and releasing aim inputs.
4. Complete all 43 placed objectives once to review the timing of the thank-you screen.
5. Confirm the connected stagger/crawl animation reads clearly for the chosen demo shot.

## Known limitations

- The loading presentation is a post-map-load UMG transition, not an asynchronous MoviePlayer screen.
- Continue is intentionally unavailable because the prototype has no save system.
- The exact wall-to-enemy shot depends on the selected camera position.
- The reaction set is limited to existing Blueprint animations and bone-name coverage.
- The visible FPS/GPU/CPU/LAT readout during validation is the machine's external performance overlay. Disable it before recording.
- External Fab/Megascans, zombie-pack, and WILDPOLY licence records are incomplete. Do not publish the package until those records are verified.

## Archive

- **ZIP path:** `deliverables/windows/DarknessOfKabul_Windows_Portfolio.zip`
- **ZIP size:** 497,824,551 bytes (474.76 MiB)
- **Entries:** 31 files
- **SHA-256:** `7FBFA86A323C3A997D12B9D1F0B252123505BD962A8242635A43387F55EC9007`
