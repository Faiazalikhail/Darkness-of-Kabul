# Darkness of Kabul - Milestone One

This branch contains only the first milestone: a controllable first-person character and an editable movement practice level in Unreal Engine 5.6.1.

## Implemented now

- walk and mouse look;
- jump;
- hold Shift to sprint;
- hold Left Ctrl to crouch and crouch-walk more slowly;
- a quiet-movement multiplier for future footsteps or AI hearing;
- press Space near a suitable low ledge to climb it;
- an editable course with a jump hurdle, crouch tunnel, three climb blocks, and floating instruction text.

The project opens directly into `/Game/Maps/L_TargetPractice`.

## Not implemented now

There is no firing input, projectile motion, damage, target logic, enemy AI, escort system, objective flow, or complete game level in milestone one. The corresponding classes remain small commented extension points for future milestones.

## Start here

1. Read [Editor Start Here](Documentation/EDITOR_START_HERE.md).
2. Read [Movement Physics and Code](Documentation/MILESTONE_ONE_MOVEMENT.md).
3. Read [Current Architecture](Documentation/PROJECT_ARCHITECTURE.md).

## Controls

| Action | Input |
|---|---|
| Move | WASD |
| Look | Mouse |
| Jump / climb nearby ledge | Space |
| Sprint | Hold Left Shift |
| Crouch / quiet walk | Hold Left Ctrl |

## Verification

`DarknessOfKabulEditor Win64 Development` builds successfully with Unreal Header Tool warnings treated as errors.
