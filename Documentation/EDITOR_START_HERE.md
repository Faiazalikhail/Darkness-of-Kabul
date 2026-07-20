# Editor and Level Guide

1. Open `DarknessOfKabul.uproject` in Unreal Engine 5.6.1.
2. Confirm the open map is `L_TargetPractice`.
3. Press **Play** and click the viewport.
4. Test WASD, mouse look, Space, Left Shift, and Left Ctrl.

## Editing the traversal area

Select `TraversalCourse` in the World Outliner. Its actor transform is editable like any other Unreal actor. It contains:

- three progressively higher jump hurdles;
- a crouch tunnel with roof and walls;
- `LowClimbBlock`, `MediumClimbBlock`, and `HighClimbBlock` as valid climbs;
- `ImpossibleClimbBlock` as a deliberately invalid climb;
- `LandingDropPlatform` for landing-impact tests;
- floating guide text for each section.

To adjust individual components visually, create a Blueprint child of `TraversalCourse` and edit the named components in the Blueprint viewport. You can also replace the cubes with ordinary level geometry while keeping the same test heights.

## Expected movement

- Walking has gradual acceleration and firm braking.
- Sprinting is faster but still controlled.
- Crouching reduces the collision capsule and uses the slowest speed.
- Space jumps when no valid ledge is detected.
- Space climbs a nearby ledge between 40 and 140 cm when the top is walkable and the capsule has room.
- The 180 cm block must reject climbing.
- Dropping from the raised platform produces a short camera impact response.

Keep this map readable as a movement and traversal test area.
