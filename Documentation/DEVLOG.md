# Development Log

This is the project diary. It records what we changed, why we changed it, what we learned, and what remains. New entries go at the top.

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
