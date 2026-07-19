# Development Log

This is the project diary. It records what we changed, why we changed it, what we learned, and what remains. New entries go at the top.

## 2026-07-19 - Playable Unreal C++ foundation

### Implemented

- Created `DarknessOfKabul.uproject`, Game and Editor targets, the runtime module, and project configuration.
- Added the complete planned class set: character, player controller, slingshot component, kinematics library, stone projectile, zombie, escort controller, stone pickup, game mode, and HUD widget contract.
- Added Enhanced Input mapping/action assets from Epic's installed UE 5.6 First Person template for movement, mouse look, controller look, and jump.
- Added direct keyboard/mouse bindings for aim, draw/release, sprint, interact, trajectory guide, and restart so the C++ character works immediately.
- Added Epic's small First Person graybox map and Level Prototyping assets as the editable starting level.
- Added a numerical automation test for the projectile example in the GDD.
- Disabled two broken, unrelated Maxon Cineware plugins that were enabled by default in the local engine installation.

### Verified

- `DarknessOfKabulEditor Win64 Development` builds successfully.
- `DarknessOfKabul Win64 Development` builds successfully.
- Unreal Header Tool generated all reflected classes without warnings-as-errors failures.
- The kinematics automation test passed: 1 succeeded, 0 warnings, 0 failures.
- The starter map and all 64 external actor packages loaded with 0 errors and 0 warnings.
- A headless game startup loaded `/Game/FirstPerson/Lvl_FirstPerson`, selected `DOKGameMode`, and brought the world up for play.

### Important student lesson

A `.umap` is a binary Unreal package, not a text file. A first attempt to create one through headless Editor Python produced a valid but empty map and triggered an Unreal 5.6 Editor crash. We removed that generated file and used the engine's validated template map instead. This is why we test assets inside Unreal rather than trusting that a file merely exists.

### Next action

Open the project, save the starter map as `/Game/Maps/DOK_Pasture`, press Play to confirm the C++ character, and begin the level graybox. Then create a Blueprint child of `ADOKCharacter` for the first-person arms and slingshot visuals.

## 2026-07-18 - Repository foundation

### What we found

- The folder was already a Git repository connected to `https://github.com/Faiazalikhail/Darkness-of-Kabul.git`.
- The repository had no commits and no tracked project files yet.
- Unreal Engine 5.6.1 and Visual Studio 2022 were already installed.
- The MSVC compiler, C++ game-development workload, Windows SDK, Git LFS, Enhanced Input, Niagara, Fab, Bridge, and Modeling Tools were available.
- The Unreal `.uproject` did not exist yet.

### What we added

- `.gitignore` for Unreal, Visual Studio, Rider, VS Code, crash, and cache files.
- `.gitattributes` for line endings and Git LFS binary patterns.
- `.editorconfig` for consistent text and Unreal-style C++ indentation.
- `.vsconfig` so teammates can reproduce the Visual Studio components.
- A project README, setup guide, architecture guide, and Unreal C++ learning notes.

### Why we did not install more plugins

The minimum project can use engine features already installed. Gameplay Ability System, StateTree, and rope simulation would create more code and debugging work without improving the required projectile-physics demonstration.

### Important Git lesson

Git was already initialized because `.git` existed. Running `git init` again would not create a new project history; the useful work was configuring ignore rules and LFS for Unreal files.

### Next action

Create the `DarknessOfKabul` C++ First Person project directly in this repository, then verify that a clean Development Editor build opens. After that, create the planned source folders through the first small class exercise.
