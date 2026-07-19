# Foundation Implementation Notes

This page explains what each implemented class contributes and what Unreal idea it teaches.

## Project and module files

| File | Student meaning |
|---|---|
| `DarknessOfKabul.uproject` | Describes the project, runtime module, engine version, and enabled/disabled plugins. |
| `DarknessOfKabul.Build.cs` | Lists engine modules our C++ code is allowed to include and link. |
| `DarknessOfKabul.Target.cs` | Rules for building the standalone game. |
| `DarknessOfKabulEditor.Target.cs` | Rules for building the Unreal Editor version of our game module. |
| `DefaultEngine.ini` | Default map, game mode, rendering, collision channels, and navigation defaults. |
| `DefaultInput.ini` | Tells Unreal to use Enhanced Player Input and the Enhanced Input component. |

## Character and input

`ADOKPlayerController` loads `/Game/Input/IMC_Default` and `/Game/Input/IMC_MouseLook`, then adds them to the local player's Enhanced Input subsystem. A **mapping context** is a collection that says which device keys produce which Input Actions.

`ADOKCharacter` binds four asset-based actions:

- `IA_Move` -> a two-dimensional movement value;
- `IA_Look` -> controller look;
- `IA_MouseLook` -> mouse look;
- `IA_Jump` -> jump start and stop.

It directly binds the project-specific keyboard/mouse keys so the first version is playable without more asset creation. It also owns the first-person camera, health, sprint/aim speeds, interaction trace, and `UDOKSlingshotComponent`.

Important includes:

- `EnhancedInputComponent.h` gives `BindAction` and trigger events.
- `InputActionValue.h` gives the typed input value passed to movement/look functions.
- `Camera/CameraComponent.h` gives the first-person camera.
- `GameFramework/CharacterMovementComponent.h` gives walk speed and movement settings.

## Slingshot component

`UDOKSlingshotComponent` is an Actor Component because the behavior belongs to the player but is reusable and does not need its own world transform.

It owns:

- current and maximum ammunition;
- draw start time and draw percentage;
- minimum/maximum launch speed;
- normal Unreal gravity;
- cooldown and projectile spawning;
- optional trajectory debug points.

It does not move the flying stone. That job belongs to the projectile actor.

## Kinematics library

`UDOKKinematicsLibrary` is a Blueprint Function Library. Its functions are static and do not need an Actor or world state. Both the trajectory preview and projectile call the same functions:

```text
Position = Start + InitialVelocity * Time + 0.5 * Acceleration * Time^2
Velocity = InitialVelocity + Acceleration * Time
```

The automation test uses the GDD example and confirms the code produces `(2000, 0, -122.5)` cm at 0.5 seconds.

## Stone projectile

`ADOKStoneProjectile` is an Actor because it needs a world position, visible mesh, owner/instigator, and lifetime.

Every Tick:

1. store the previous world position;
2. add Delta Time to total flight time;
3. calculate the new analytical position;
4. sweep a 3 cm sphere from old to new position;
5. resolve a hit or move to the new position;
6. rotate toward the calculated velocity;
7. destroy after five seconds.

It intentionally does not use `UProjectileMovementComponent`, because the course needs our C++ kinematics to be visible and explainable.

## Zombie

`ADOKZombie` extends `ACharacter` because it needs character movement and an AI Controller. The small `EDOKZombieState` enum keeps the required behavior readable: Idle, Walk, Stagger, Attack, Dead.

It requests navigation movement toward its target, attacks inside range, staggers after a surviving hit, and disables movement/collision on death. Visual mesh and animation remain Blueprint work.

## Pickup, escort, game mode, and HUD

- `ADOKStonePickup` blocks the interaction trace and adds ammo only when capacity is available.
- `ADOKEscortController` stores stage objectives and flock danger, then asks the Game Mode to win or lose.
- `ADOKGameMode` selects our pawn/controller and owns level-wide state, objective, win, loss, and restart.
- `UDOKHUDWidget` is an abstract C++ contract. A Widget Blueprint will implement its visual events without owning gameplay rules.

## What is deliberately unfinished

The foundation compiles and runs, but visual production belongs in the Editor:

- first-person arms and Y-shaped slingshot mesh;
- draw and release animations;
- Blueprint HUD layout;
- zombie skeletal mesh/animations and bone naming check;
- the real five-zone map;
- livestock/dog assets and waypoint presentation;
- sound, Niagara effects, final collision presets, and tuning.

Those are the correct next tasks because they depend on visual assets and level decisions, not missing foundation code.
