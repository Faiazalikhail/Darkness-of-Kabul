# Darkness of Kabul

A first-person Unreal C++ slingshot prototype featuring ricochet prediction, projectile bounce, pursuing enemies, and physical enemy reactions.

## Current prototype status

The target-practice map supports repeatable slingshot shots against environment surfaces and bone-aware zombie targets. Zombies detect the player by sight, chase, and attack, and a struck zombie raises a disturbance that pulls only its immediate neighbours. The current beta adds a complete UMG shell, two-contact ricochets, objective tracking, player health with a game over state, and a replayable prototype ending.

## Verified contribution

The repository contains a code-driven first-person controller, charge-and-release slingshot, two-impact ricochet prediction, swept bouncing projectile, body-area damage reactions, ragdoll death, navigation-driven enemy pursuit with radius-limited alerting, player health and death handling, objective/reset flow, and native UMG screens. Blueprint children provide the verified player, projectile, zombie, and target presentation assets.

## Prototype flow

The packaged build opens with a loading screen, a press-any-button welcome screen, and a main menu. New Game starts the target practice; Continue is visibly unavailable for this prototype. Settings include master volume, look sensitivity, field of view, and display mode. Escape opens the pause menu. Defeating every zombie and striking every wobble target opens the prototype-beta thank-you screen with Play Again and Leave actions. Letting the zombies reach the player ends the run on a game over screen.

## Enemy behaviour

Zombies sense on a staggered per-zombie timer using range, a forward vision cone, and a line-of-sight trace, so walls break contact. Striking a zombie raises a disturbance that alerts only those within its radius, so hitting one of a group pulls its neighbours rather than the whole level. Alerted zombies close distance, stop at attack range, and swing on an interval. A zombie that ends up far enough away gives up and returns to unaware. Torso hits stagger a standing zombie, leg hits force crawling, and a downed zombie stays down.

The aim preview deliberately avoids drawing a full flight line. A zombie impact is marked on the body. Surface impacts show only a short incoming/outgoing angle and a final impact marker, including one-wall indirect shots. The real stone and predictor both stop after at most two collisions.

## Technology

- **Engine:** Unreal Engine 5.6.1
- **Language:** C++
- **Platform:** Windows
- **Focus:** gameplay systems, enemy reactions, and UMG integration
- **Workflow:** Unreal Editor, verified Blueprints, Visual Studio, Git

## Controls

| Action | Input |
|---|---|
| Move / look | WASD / mouse |
| Aim | Hold right mouse button |
| Pull and fire | Hold left mouse button, then release |
| Reset player position, stones, enemies, and objectives (asks first) | R |
| Pause / resume | Escape |
| Jump / climb | Space |
| Sprint / crouch | Left Shift / Left Ctrl |

## Run the packaged build

Open `deliverables/windows/DarknessOfKabul_Windows_Portfolio/` and run `DarknessOfKabul.exe`.

## Build from source

Open `DarknessOfKabul.uproject` in Unreal Engine 5.6.1. Build the `DarknessOfKabul` Win64 Shipping target, then package for Windows. The cooked startup map is `/Game/Maps/L_TargetPractice`.

## Documentation

- [Editor and playtest guide](Documentation/EDITOR_START_HERE.md)
- [Project architecture](Documentation/PROJECT_ARCHITECTURE.md)
- [Portfolio notes](PORTFOLIO_NOTES.md)
- [Asset credits](Documentation/ASSET_CREDITS.md)

## Tested Windows configuration

Windows 10 SDK 10.0.22621.0, Visual Studio 2022 toolchain 14.38.33145, Unreal Engine 5.6.1, and the Win64 Shipping target.

## Known limitations

- Holding a pulled shot for 3.5 seconds triggers the original overdraw camera kick, cancels the pull, and requires the player to aim again.
- The loading presentation runs after the target-practice map has loaded; it is a short UMG transition, not an asynchronous movie-player loading screen.
- The prototype has one tuned slingshot interaction and no save system; Continue is intentionally disabled.
- Hit zones depend on Physics Asset bone names; unknown bones fall back to the torso.
- Reaction animation coverage is limited to the existing Blueprint assets.
- Enemy pursuit requires a navigation mesh in the level; zombies cannot move without one.
- Volume, look sensitivity, and field of view reset to defaults on relaunch. Only display mode persists.
- The packaged scene requires a short hands-on check for the preferred wall-to-enemy camera angle.
- External asset licence records are incomplete, so the package should not be published until those records are verified.

## Credits and licences

Epic Games template and engine content is used under the applicable Unreal Engine licence. The project also contains a zombie animation pack, Fab/Megascans content, and a WILDPOLY animal pack whose source and licence records are not complete in the repository. See `Documentation/ASSET_CREDITS.md`; verify those entries before public redistribution.
