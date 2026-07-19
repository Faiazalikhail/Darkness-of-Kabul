# Darkness of Kabul: The Road to Home

A small first-person Unreal Engine 5.6.1 level for **VGP203 - Physics for Game Development**. The main graded feature is a Y-shaped slingshot whose stone trajectory, velocity, gravity, and swept collision are calculated in C++.

> Current stage: the Unreal 5.6 C++ project, complete foundation class set, Enhanced Input assets, and editable First Person graybox map are implemented and build successfully.

## Minimum project goal

In one 10-15 minute level, the player escorts livestock from a pasture to a home courtyard while using a limited supply of stones against simple zombies. The minimum build includes one slingshot, one stone type, one zombie type, a linear five-zone route, win/loss conditions, restart, and a packaged Windows build.

## Start here

1. Read [Setup and Workflow](Documentation/SETUP_AND_WORKFLOW.md).
2. Follow [Your First Unreal Editor Session](Documentation/EDITOR_START_HERE.md).
3. Read [Project Architecture](Documentation/PROJECT_ARCHITECTURE.md).
4. Use [Unreal C++ Learning Notes](Documentation/UNREAL_CPP_LEARNING_NOTES.md) whenever we add a class.
5. Check [Development Log](Documentation/DEVLOG.md) to see what changed and why.
6. Keep the original [Game Design Document](Documentation/Darkness%20of%20Kabul%20GDD.docx) as the design source of truth.

## Repository rule

Commit source files, configuration, documentation, and game assets. Do not commit Unreal-generated `Binaries`, `Intermediate`, `Saved`, or `DerivedDataCache` folders. Large binary assets are handled by Git LFS.

## Controls planned by the GDD

| Action | Input |
|---|---|
| Move / Look | WASD / Mouse |
| Aim | Right Mouse Button |
| Draw and release | Hold / release Left Mouse Button |
| Collect stone | E |
| Sprint | Left Shift |
| Trajectory guide | T |
| Restart | R |
| Pause | Escape |

## Team definition of done

- The project builds from Visual Studio and opens in Unreal Engine 5.6.1.
- The real stone and trajectory guide use the same C++ equations.
- A swept sphere trace prevents fast stones from passing through thin targets.
- Head and body hits produce different results.
- The player can complete or fail the pasture-to-home route and restart.
- A packaged Windows build runs without the Unreal Editor.
