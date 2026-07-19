# Unreal C++ Learning Notes

Use this as our growing translation guide from normal C++ to Unreal C++. Every time we add a script/class, we should update this file with the new Unreal ideas used in that class.

## The main mental model

Regular C++ still matters: classes, inheritance, pointers, references, `const`, functions, headers, source files, and access control all behave normally. Unreal adds a reflection system, garbage-collected objects, engine-managed actors/components, editor-visible properties, and a build tool that scans special macros.

## Names that look strange at first

| Prefix | Meaning | Project example |
|---|---|---|
| `A` | Actor that can exist in a level/world | `ADOKStoneProjectile` |
| `U` | `UObject`-based object or component | `UDOKSlingshotComponent` |
| `F` | Plain Unreal value type or struct | `FVector`, `FHitResult` |
| `E` | Enum | `EDOKZombieState` |
| `I` | Interface | only add if we truly need one |
| `T` | Template/container type | `TArray<FVector>` |

These prefixes are part of Unreal’s naming and reflection expectations, not decoration.

## Reflection macros

```cpp
UCLASS()
class DARKNESSOFKABUL_API ADOKStoneProjectile : public AActor
{
	GENERATED_BODY()
};
```

- `UCLASS()` tells Unreal Header Tool that this is a reflected class.
- `GENERATED_BODY()` inserts code Unreal needs for reflection and object creation.
- `DARKNESSOFKABUL_API` exports the class from our game module. The module name is converted to uppercase for this macro.
- A reflected header must include its matching `.generated.h` file last.

The generated file does not exist the first time we write the header. Unreal Build Tool creates it during the build.

## Properties

```cpp
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Slingshot|Physics")
float MaxDrawTime = 1.25f;
```

- `UPROPERTY` lets Unreal see and manage the member.
- `EditDefaultsOnly` allows editing on a Blueprint class/default, not separately on every placed instance.
- `BlueprintReadOnly` allows Blueprint graphs to read it but not change the rule directly.
- `Category` keeps the Details panel organized.
- The `f` in `1.25f` makes the literal a `float`.

Do not expose every variable. Expose tuning values; keep internal state private.

## Functions

```cpp
UFUNCTION(BlueprintCallable, Category = "Slingshot")
void StartDrawing();
```

`BlueprintCallable` makes a C++ function available as a Blueprint node. Pure math functions may use `BlueprintPure` when they do not change state.

## Actors versus components

An `AActor` has a world identity and transform. A `UActorComponent` is reusable behavior attached to an actor. The stone is an actor because it exists and moves in the world. The slingshot logic is a component because it belongs to the player and does not need to be a separate level object.

## Unreal object pointers

For reflected object members, we will normally use:

```cpp
UPROPERTY(VisibleAnywhere)
TObjectPtr<USceneComponent> Root;
```

`TObjectPtr` helps Unreal track object references. For temporary local access, a normal pointer such as `UWorld* World` is common. Never create an actor with normal C++ `new`; use Unreal spawning functions such as `GetWorld()->SpawnActor`.

## Constructor, BeginPlay, and Tick

- **Constructor:** create default subobjects and set default values. The world may not be ready.
- **BeginPlay:** runs when gameplay begins; safe for runtime initialization.
- **Tick:** runs every frame if enabled. It receives `DeltaTime`, the number of seconds since the previous frame.

Our stone needs Tick for its custom flight update. Most other classes should avoid Tick unless they truly need it.

## Header includes and forward declarations

An include copies declarations from another header into the current file. Too many includes slow compilation and create dependency problems.

In a header, prefer a forward declaration when only a pointer/reference is stored:

```cpp
class USphereComponent;
```

Then include the full header in the `.cpp`:

```cpp
#include "Components/SphereComponent.h"
```

You need the full include in the header when inheriting from a type or storing it by value. `CoreMinimal.h` provides common Unreal core types, but it does not replace every specific engine header.

## Build.cs is part of C++ dependency management

An include may be correct and still fail if its module is missing from `DarknessOfKabul.Build.cs`. Think of it as two permissions:

1. `#include` tells the compiler which declaration this file needs.
2. `Build.cs` tells Unreal Build Tool which compiled module the whole game depends on.

We will add dependencies only when needed and record why.

## FVector and Unreal units

`FVector` stores X, Y, and Z. Unreal normally uses centimetres:

- position: cm;
- velocity: cm/s;
- acceleration: cm/s²;
- time: seconds;
- rotation: degrees in `FRotator`.

For the projectile, gravity is `FVector(0.0, 0.0, -980.0)` and the position formula returns an `FVector`.

## Collision traces

A trace asks the world whether a shape travelling from A to B hits something. We use a sphere trace instead of checking only the new stone position. The result is an `FHitResult` containing data such as actor, component, impact point, normal, physical material, and bone name.

Collision **channels** answer “what kind of query is this?” Object types answer “what kind of thing is this?” We will define a clear stone/projectile response matrix instead of letting every asset use random defaults.

## Logging and debugging

```cpp
UE_LOG(LogTemp, Display, TEXT("Launch speed: %.1f cm/s"), LaunchSpeed);
```

`TEXT()` creates an Unreal-compatible string literal. Logs are useful for values, but `DrawDebugSphere` and `DrawDebugLine` are often better for understanding a trajectory in 3D space. Debug drawing must be removable or disabled for the final build.

## Rule for every new class

When we add a class, document:

1. why this class exists;
2. why it is an Actor, Component, UObject, Widget, or plain struct;
3. its important includes and Build.cs module;
4. each new Unreal macro;
5. what C++ owns and what Blueprint owns;
6. how we test it in isolation;
7. what can go wrong and how we will see the error.
