# Milestone-One Foundation

## Implemented source

- `PlayerCharacter`: first-person locomotion and collision-safe ledge climbing.
- `KabulGameMode`: chooses the movement character and player controller.
- `TraversalCourse`: editor-placeable movement blockout with floating text.

## Existing future stubs

The repository still contains small placeholder classes for the slingshot, stone projectile, pickup, target/enemy, and HUD. They have no firing input or active gameplay behavior. Their comments show where later milestones can begin without mixing those systems into movement code.

## Level ownership

The large floor and lighting are normal actors stored in `L_TargetPractice`. `TraversalCourse` is also stored in the map and can be moved with Unreal's normal tools. C++ only supplies a clean starting arrangement of named components.

## Milestone-one acceptance checklist

- The project opens in `L_TargetPractice`.
- WASD and mouse look work.
- Jump, sprint, crouch, slower crouch walking, and ledge climbing work.
- Crouch exposes a lower movement-noise multiplier.
- There is no left-click firing behavior.
- The course and floating text are visible and editable in Unreal.
- The Editor development target compiles successfully.
