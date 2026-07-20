# Darkness of Kabul

A first-person movement and traversal physics project built with Unreal Engine 5.6.1 and C++.

## Current gameplay

- WASD movement and mouse look;
- hold Shift to sprint;
- hold Left Ctrl to crouch and move quietly;
- Space to jump or climb a nearby valid ledge;
- heavier acceleration, jumping, air control, and landing feedback;
- an editable traversal area for jump, crouch, climb, blocked-climb, and landing tests.

The project opens directly into `/Game/Maps/L_TargetPractice`.

## Start here

1. [Open and test the level](Documentation/EDITOR_START_HERE.md).
2. [Understand input and movement physics](Documentation/MOVEMENT_AND_INPUT.md).
3. [Review the project architecture](Documentation/PROJECT_ARCHITECTURE.md).
4. [Learn the Unreal C++ concepts](Documentation/UNREAL_CPP_LEARNING_NOTES.md).

## Controls

| Action | Input |
|---|---|
| Move | WASD |
| Look | Mouse |
| Jump / climb nearby ledge | Space |
| Sprint | Hold Left Shift |
| Crouch / quiet walk | Hold Left Ctrl |

## Build verification

`DarknessOfKabulEditor Win64 Development` builds successfully with Unreal Header Tool warnings treated as errors.
