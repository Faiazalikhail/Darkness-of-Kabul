# Milestone One: Editor Start

1. Open `DarknessOfKabul.uproject` in Unreal Engine 5.6.1.
2. Confirm the map tab is `L_TargetPractice`.
3. Press **Play** and click the viewport.
4. Test WASD, mouse look, Space, Left Shift, and Left Ctrl.

## Editing the course

Select `TraversalCourse` in the World Outliner. Its actor transform is editable like any other Unreal actor. The course contains named components:

- `JumpHurdle`;
- `CrouchTunnelRoof` and two tunnel walls;
- `LowClimbBlock`, `MediumClimbBlock`, and `HighClimbBlock`;
- `GuideText`.

The floating text is deliberately preserved. To edit individual course components visually, create a Blueprint child of `TraversalCourse` and adjust the named components in the Blueprint viewport. You may also replace any component with ordinary cubes or other level objects.

Keep this map as a readable movement test. Do not add projectiles, enemies, objectives, animals, or story events during milestone one.

## Expected movement

- Sprint is faster than normal walking.
- Crouching reduces the collision capsule and uses the slowest speed.
- Crouch movement exposes the lowest noise multiplier for future audio or AI work.
- Space jumps normally when no ledge is detected.
- Space climbs when a nearby ledge is within the configured 40-140 cm range and has enough landing clearance.
