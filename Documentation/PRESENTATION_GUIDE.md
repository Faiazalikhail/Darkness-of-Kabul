# Darkness of Kabul — Technical Walkthrough and Presentation Guide

A study-then-present guide to the gameplay and physics systems.

**How to use this document.** Read Parts 1–6 once slowly to learn the systems. Then present from Part 7, which is a timed script. Part 8 prepares you for questions. Every number and code path below was read directly from the source, so you can state them with confidence.

---

## Part 1 — The one-minute summary

> Darkness of Kabul is a first-person slingshot prototype built in Unreal Engine 5.6 with C++. The player charges a shot, releases a stone, and the stone obeys projectile physics with gravity, bounce, and friction. The interesting part is that the aim preview runs the *same* physics as the real stone, so what the preview shows is what actually happens — including ricochets off walls onto enemies behind cover. Enemies react based on which bone the stone actually struck.

That paragraph is your opening. Everything else is detail supporting it.

**The three things that make this project non-trivial:**

1. **Prediction and reality share one physics model.** Most prototypes fake the preview line. This one re-simulates the real equations.
2. **Damage is resolved from the physics skeleton**, not a hitbox approximation. The struck bone name decides the reaction.
3. **The projectile is capped at two contacts** by design, and both the real stone and the predictor enforce that cap identically.

---

## Part 2 — Architecture map

Six systems, each with one job.

| System | File | Responsibility |
|---|---|---|
| `APlayerCharacter` | `Private/PlayerCharacter.cpp` | Movement, slingshot state, launch, reset, owns the UI |
| `UKinematicsLibrary` | `Private/KinematicsLibrary.cpp` | Pure charge → speed maths, no state |
| `USlingshotAimGuideComponent` | `Private/SlingshotAimGuideComponent.cpp` | Re-simulates flight to find impacts |
| `AStoneProjectile` | `Private/StoneProjectile.cpp` | The real flying stone, bounce, damage delivery |
| `AZombieCharacter` | `Private/ZombieCharacter.cpp` | Bone → zone → damage → reaction → ragdoll |
| `AKabulGameMode` | `Private/KabulGameMode.cpp` | Objective tracking and completion |
| `UKabulPrototypeUI` | `Private/KabulPrototypeUI.cpp` | All screens plus impact-marker drawing |

**Data flow of a single shot:**

```
Hold RMB ──► bIsAiming = true
Hold LMB ──► bChargingStone = true, ChargeStartTime recorded
   │
   ├── every frame ──► GetSlingshotTrajectory() ──► predictor ──► UI markers
   │
Release LMB ──► held time ──► CalculateLaunchSpeed() ──► FireStone()
                                                            │
                                              AStoneProjectile spawned + launched
                                                            │
                                              bounce ──► zombie? ──► ReceiveStoneImpact()
                                                            │
                                              bone name ──► hit zone ──► damage ──► reaction
                                                            │
                                              death ──► ragdoll + GameMode notified
```

**Why the design is split this way.** The charge maths lives in a static library so both the preview and the real launch call the identical function — they cannot drift apart. The predictor is a component rather than code inside the player so it can be tested and reused. Rendering lives in the UI layer so the Shipping build never depends on debug drawing.

---

## Part 3 — The physics core (spend most of your time here)

### 3.1 Charge to launch speed

`KinematicsLibrary.cpp` — two pure functions, no side effects.

```cpp
float UKinematicsLibrary::CalculateChargeAmount(HeldTime, MinTime, MaxTime)
{
    const float ChargeRange = FMath::Max(MaxTime - MinTime, KINDA_SMALL_NUMBER);
    return FMath::Clamp((HeldTime - MinTime) / ChargeRange, 0.0f, 1.0f);
}
```

This normalises how long the button was held into a 0–1 value. The `FMath::Max` guard prevents a divide-by-zero if the designer sets both times equal. The `Clamp` means holding longer than the maximum cannot exceed full power.

```cpp
float UKinematicsLibrary::CalculateLaunchSpeed(...)
{
    return FMath::Lerp(MinSpeed, MaxSpeed, CalculateChargeAmount(...));
}
```

A linear interpolation from the normalised charge.

**The numbers:** minimum charge `0.3 s`, maximum charge `2.5 s`, speed range **6,000 → 9,200 cm/s** (60–92 m/s).

**Say this:** "Charge time maps linearly to launch speed. It is deliberately one shared function so the preview and the real shot can never disagree."

### 3.2 Integration — how the stone moves through time

This is the heart of the predictor, in `SlingshotAimGuideComponent.cpp`:

```cpp
const FVector NextVelocity = Velocity + Gravity * TimeStep;
const FVector NextPosition = Position + (Velocity + NextVelocity) * (0.5f * TimeStep);
```

This is the **trapezoidal rule**, and for constant acceleration it is *exact*, not an approximation. Expand it algebraically:

```
NextPosition = P + (V + (V + g·dt)) · dt/2
             = P + (2V + g·dt) · dt/2
             = P + V·dt + ½·g·dt²
```

That final line is the standard kinematic equation **s = ut + ½at²**.

**Say this — it is your strongest physics point:** "I used trapezoidal integration rather than simple Euler. Because gravity is constant, this reduces exactly to s = ut + ½at², so the predicted path carries no integration error from the maths itself. Plain Euler would drift low on every step and the preview would slowly disagree with the real shot."

If asked why not just use Euler: Euler (`P += V·dt` before updating V) systematically under-shoots on a falling arc, and the error compounds every step. Over an 8-second flight that is a visible gap.

**Gravity** is read from the world, not hard-coded:

```cpp
const FVector Gravity(0.0f, 0.0f, World->GetGravityZ() * ProjectileGravityScale);
```

So if the world gravity changes, prediction follows automatically.

### 3.3 The bounce — impulse-based collision response

The most technical function in the project:

```cpp
FVector CalculateBounceVelocity(FVector Velocity, const FVector& SurfaceNormal,
                                const float Bounciness, const float Friction)
{
    const float VelocityDotNormal = FVector::DotProduct(Velocity, SurfaceNormal);
    if (VelocityDotNormal > 0.0f) { return Velocity; }      // already moving away

    const FVector ReflectedNormal = SurfaceNormal * -VelocityDotNormal;
    Velocity += ReflectedNormal;                             // cancel normal component

    const float TangentialSpeed = Velocity.Size();
    const float ScaledFriction = TangentialSpeed > KINDA_SMALL_NUMBER
        ? FMath::Clamp(-VelocityDotNormal / TangentialSpeed, 0.0f, 1.0f) * Friction
        : Friction;

    Velocity *= FMath::Clamp(1.0f - ScaledFriction, 0.0f, 1.0f);   // slow the slide
    Velocity += ReflectedNormal * FMath::Max(Bounciness, 0.0f);    // restitution
    return Velocity;
}
```

**Walk through it in four steps — this is exactly how to explain it:**

1. **Decompose.** `V · N` gives the component of velocity along the surface normal. Negative means approaching the surface. If positive, the stone is already leaving and nothing should happen — that guard prevents a stone stuck vibrating inside geometry.
2. **Remove the normal component.** `ReflectedNormal` has magnitude `|V·N|` pointing away from the wall. Adding it to `V` cancels the into-the-wall part, leaving only the **tangential** (sliding-along-the-wall) part.
3. **Apply friction to the tangent only.** Friction slows sliding, not bouncing. The scale factor `-V·N / |tangential|` means a *head-on* hit gets more friction than a *glancing* hit — physically, a steep impact bites into the surface while a shallow one skids.
4. **Add restitution along the normal.** `ReflectedNormal * Bounciness` puts back an outgoing normal velocity equal to `0.32 ×` the incoming. That coefficient **0.32** is the coefficient of restitution *e* — a stone keeps about a third of its perpendicular speed.

**Say this:** "Collision response separates velocity into normal and tangential components. Restitution acts on the normal, friction acts on the tangent, and the friction strength scales with impact angle. Bounciness 0.32 is the coefficient of restitution — a fairly dead stone, which is what you want so ricochets stay readable."

**Values:** Bounciness `0.32`, Friction `0.28`, `bBounceAngleAffectsFriction = true`.

### 3.4 Why the prediction actually matches

This is the design decision worth defending. Four separate things are deliberately aligned:

| Aspect | Real stone (`AStoneProjectile`) | Predictor |
|---|---|---|
| Bounce maths | `UProjectileMovementComponent` | `CalculateBounceVelocity` — same terms |
| Timestep | `MaxSimulationTimeStep = 0.025` | `1 / SimulationFrequency` = `1/40` = **0.025** |
| Collision shape | Sphere radius `5.0` | `FCollisionShape::MakeSphere(Radius)` from the stone |
| Collision channel | `StoneProjectile` profile | `ECC_GameTraceChannel1` — same channel |
| Stop threshold | `BounceVelocityStopSimulatingThreshold = 250` | `BounceVelocity.SizeSquared() < 250²` |

The predictor does not use its own constants. It reads them from the projectile's class-default object at call time:

```cpp
const AStoneProjectile* ProjectileDefaults = StoneProjectileClass->GetDefaultObject<AStoneProjectile>();
return SlingshotAimGuide->PredictTrajectory(
    LaunchLocation, LaunchDirection * PreviewSpeed,
    ProjectileDefaults->GetCollisionRadius(),
    ProjectileDefaults->GetProjectileGravityScale(),
    ProjectileDefaults->GetBounciness(),
    ProjectileDefaults->GetFriction(),
    ProjectileDefaults->GetMaximumCollisionCount(),
    ProjectileDefaults->GetMaximumLifetime(),
    this, OutPrediction);
```

**Say this:** "If a designer retunes the stone's bounciness in the editor, the preview updates automatically, because the predictor asks the projectile for its values rather than storing copies. There is no second set of constants to keep in sync."

Note `SizeSquared()` compared against `250²` — comparing squared magnitudes avoids a square root per step. A small but real optimisation inside the simulation loop.

The predictor also uses a **swept sphere** (`SweepSingleByChannel`), not a point raycast. A point trace would slip through thin geometry and past corners the real 5 cm stone would clip.

### 3.5 The two-contact rule

Enforced on both sides. In the predictor:

```cpp
if (Impact.bHitZombie
    || OutPrediction.Impacts.Num() >= MaximumCollisionCount
    || BounceVelocity.SizeSquared() < FMath::Square(250.0f))
{
    return true;   // stop simulating
}
```

In the real stone:

```cpp
if (ProjectileMovement && CollisionCount >= MaximumCollisionCount)
{
    ProjectileMovement->StopSimulating(ImpactResult);
}
```

A zombie body hit is always terminal — the stone stops there:

```cpp
// A body hit is the final collision. This keeps the real stone and
// the body marker in the preview in exact agreement.
ProjectileMovement->StopSimulating(ImpactResult);
```

`MaximumCollisionCount = 2`. So the legal shapes are: direct hit, or one wall bounce then a second and final impact. That second impact is what makes **indirect ricochet kills** possible — the skill shot of the prototype.

**Why cap it at two?** Readability. An unlimited bouncing stone is unpredictable, so the preview would be useless and the player could not aim deliberately. Two contacts is enough for a wall-bank shot while staying fully previewable.

---

## Part 4 — Gameplay systems

### 4.1 Slingshot state and the overdraw rule

Two booleans hold the whole state: `bIsAiming` (right mouse) and `bChargingStone` (left mouse). Release below `MinimumChargeTime` (0.3 s) cancels the shot entirely.

The overdraw rule, checked every tick:

```cpp
void APlayerCharacter::UpdateSlingshotAim()
{
    if (!bIsAiming || !bChargingStone || GetSlingshotHeldTime() < MaximumDrawDuration)
    {
        return;
    }

    // Preserve the original overdraw consequence: the pull fails, aim resets,
    // and the camera kick tells the player to release before aiming again.
    bChargingStone = false;
    bIsAiming = false;
    LandingShakeElapsed = 0.0f;
    LandingShakeStrength = 0.65f;
}
```

Hold past **3.5 seconds** and the pull fails: charge cancelled, aim dropped, camera kicked. The player must re-aim.

**Say this:** "This is a deliberate cost for over-holding. Maximum power arrives at 2.5 seconds, so there is a one-second window to release. Greedy holding loses the shot. It creates tension in what would otherwise be a free, risk-less charge."

### 4.2 Hit zones — resolving damage from the physics skeleton

Damage is not decided by a box. It is decided by the **bone the physics asset reports**:

```cpp
EZombieHitZone AZombieCharacter::ClassifyHitBone(const FName BoneName) const
{
    const FString Bone = BoneName.ToString().ToLower();

    if (Bone.Contains("head") || Bone.Contains("neck"))       return EZombieHitZone::Head;
    if (Bone.Contains("thigh") || Bone.Contains("calf")
        || Bone.Contains("foot") || Bone.Contains("ball"))    return EZombieHitZone::Leg;
    if (Bone.Contains("arm") || Bone.Contains("hand")
        || Bone.Contains("clavicle"))                          return EZombieHitZone::Arm;

    // Pelvis, spine, and any future unclassified central body default here.
    return EZombieHitZone::Torso;
}
```

There is a **two-stage fallback** for robustness. If the hit result carries no bone name, the closest physical bone is found first:

```cpp
if (ResolvedHit.BoneName.IsNone() && GetMesh())
{
    ResolvedHit.BoneName = GetMesh()->FindClosestBone(...);
}
```

and any unrecognised name still resolves to Torso. **A hit can never be lost.**

**Damage table:** Head = instant kill (`Damage = CurrentHealth`), Torso `30`, Leg `20`, Arm `15`, from `MaxHealth = 100`.

**Say this:** "Because zones come from the physics asset, hit detection is exactly as accurate as the ragdoll skeleton — no separate hitbox to drift out of alignment. Head is lethal by design, not by damage value, so it works no matter how much health remains."

### 4.3 Reaction states and ragdoll

Four states: `Standing`, `Staggered`, `Crawling`, `Dead`.

- **Torso** → `BeginStagger()`, a 2-second timer, then restore the previous state
- **Leg** → `EnterCrawling()`, movement speed drops to `80`
- **Head or health ≤ 0** → `Die()`

Stagger remembers what it interrupted (`StateBeforeStagger`), so staggering a crawling zombie returns it to crawling — not to standing. A small detail that prevents an obvious bug.

Death converts the character into a physics ragdoll:

```cpp
ZombieMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
ZombieMesh->SetCollisionProfileName(TEXT("Ragdoll"));
ZombieMesh->SetAllBodiesSimulatePhysics(true);
ZombieMesh->SetSimulatePhysics(true);
ZombieMesh->WakeAllRigidBodies();

const FVector Impulse = ImpactVelocity.GetSafeNormal() * DeathImpulse;
ZombieMesh->AddImpulseAtLocation(Impulse, Hit.ImpactPoint, Hit.BoneName);
```

**Order matters, and this is worth saying:** movement is stopped, the capsule collision is disabled, then the mesh is detached from the capsule before simulating. Skipping the detach leaves the ragdoll fighting its parent capsule and it visibly jitters or drags.

The impulse (`DeathImpulse = 4500`) is applied **along the stone's travel direction, at the exact impact point, on the exact bone**. So the body falls the way it was actually struck — a headshot from the left throws the head left.

### 4.4 Objectives and reset

The game mode discovers objectives at `BeginPlay` rather than hard-coding counts — add a zombie to the level and the total updates automatically.

Progress uses **sets of actor references**, not integer counters:

```cpp
TSet<TWeakObjectPtr<AActor>> DefeatedZombies;
TSet<TWeakObjectPtr<AActor>> HitWobbleTargets;
```

**Say this:** "Sets rather than counters means hitting the same target twice cannot falsely complete another objective. It is idempotent by construction." That is a genuinely good engineering answer.

`R` restores everything in one pass without reloading the level: player transform, control rotation, camera, all live stones destroyed, every zombie's reaction state reset, and objective progress cleared. `TWeakObjectPtr` is used throughout so destroyed actors cannot leave dangling pointers.

---

## Part 5 — The interface layer

All screens are built in native C++ (`KabulPrototypeUI.cpp`, ~930 lines): loading, welcome, main menu, settings, gameplay HUD, pause, completion.

The important presentation decision is **what is deliberately not drawn**. There is no full trajectory line anywhere in the codebase. Instead:

- **Zombie hit** → a body marker only, no lines at all
- **Surface hit** → a short incoming angle piece, a short outgoing piece, and the final impact marker

```cpp
Impact.IncomingGuidePoint = ImpactPoint - IncomingDirection * AngleGuideLength;
Impact.OutgoingGuidePoint = ImpactPoint + BounceVelocity.GetSafeNormal() * AngleGuideLength;
Impact.bHasOutgoingSegment = !Impact.bHitZombie
    && OutPrediction.Impacts.Num() < MaximumCollisionCount
    && !BounceVelocity.IsNearlyZero();
```

`AngleGuideLength = 55` units — just enough to read the angle.

**Say this:** "A full line would tell the player everything and remove the skill. Showing only the bounce *angle* and the final impact point communicates enough to aim a ricochet deliberately, while still requiring the player to understand how the angle works."

Note also that the outgoing piece is suppressed when the stone has used its last allowed contact — the preview never promises a bounce that will not happen.

---

## Part 6 — Engineering practices worth mentioning

- **Blueprint-friendly by design.** `BP_OnZombieHit`, `BP_OnPhysicalStateChanged`, and `BP_OnZombieDeath` are `BlueprintImplementableEvent`s, so animation and effects are authored in Blueprint while all rules stay in C++.
- **Designer-tunable.** Speeds, damages, durations, and thresholds are `UPROPERTY(EditDefaultsOnly)` with `ClampMin` guards, so they can be retuned in-editor without recompiling and cannot be set to invalid values.
- **Defensive coding throughout.** Null checks on every world/component access, `GetSafeNormal()` instead of raw normalisation, `KINDA_SMALL_NUMBER` guards before division.
- **Validated build.** Editor Development and Shipping targets both compile; all Blueprints compile with 0 errors, 0 warnings, 0 failed loads.

---

## Part 7 — Presentation script (about 8 minutes)

**Setup before you start:** packaged build open, external FPS/GPU overlay disabled, a wall-bank angle already rehearsed.

| Time | What you say | What you do |
|---|---|---|
| 0:00–0:45 | The one-minute summary from Part 1 | Main menu on screen |
| 0:45–1:30 | "Charge maps linearly to launch speed, 6,000 to 9,200 cm/s, through one shared function." | New Game; hold LMB, show orange → green |
| 1:30–3:00 | **Integration.** Trapezoidal rule reduces exactly to s = ut + ½at². | Aim at a wall, show the angle preview |
| 3:00–4:30 | **Bounce maths.** Normal vs tangential, restitution 0.32, friction 0.28 scaled by angle. | Fire the ricochet |
| 4:30–5:30 | "Prediction and reality share timestep, radius, channel, and constants — read from the projectile itself." | Show preview matching the real path |
| 5:30–6:30 | **Hit zones from the physics skeleton.** Head lethal, torso stagger, leg crawl. | Land a torso hit, hold 2 s; then a leg hit |
| 6:30–7:15 | **Ragdoll.** Detach, simulate, impulse along travel direction at the struck bone. | Land a headshot |
| 7:15–8:00 | Objectives as sets, full scenario reset | Press `R`, show `SCENARIO RESET`, open pause |

**If you only get three minutes:** do the one-minute summary, the bounce maths (3.3), and one ricochet kill. That is the strongest core.

---

## Part 8 — Questions you should expect

**"Why not use Unreal's built-in `PredictProjectilePath`?"**
It does not model bounce with friction and restitution the way `ProjectileMovementComponent` does, so the preview would diverge from the real stone exactly where it matters — after the first bounce. Re-implementing the same bounce terms is what makes the two agree.

**"Is your integration method accurate?"**
For constant acceleration it is exact. The trapezoidal step expands algebraically to `s = ut + ½at²`. Any error comes from the collision timestep granularity, not from the integrator.

**"What is the timestep and why?"**
0.025 s, i.e. 40 Hz, chosen to match the projectile's `MaxSimulationTimeStep` of 0.025 exactly. Aligning them is why prediction tracks reality.

**"Why squared comparisons?"**
`SizeSquared()` against `250²` avoids a square root inside the per-step loop. Same result, less cost.

**"What is 0.32?"**
The coefficient of restitution — the stone retains 32% of its normal-direction speed after impact. Low, so ricochets stay short and readable.

**"How does friction depend on angle?"**
The scale is `|V·N| / |tangential velocity|`, so a head-on impact applies close to full friction and a glancing impact applies little. That matches a real stone skidding off a shallow surface.

**"Why cap collisions at two?"**
Predictability. Unlimited bounces cannot be previewed usefully, so the player could not aim deliberately. Two allows a bank shot while staying fully previewable.

**"What happens if the bone name is unknown?"**
Two fallbacks: nearest physical bone, then Torso. A hit is never dropped.

**"What would you do next?"**
Directional hit reactions selected from impact angle, and a zombie animation graph driven by the existing physical-state events. The C++ hooks for both already exist.

---

## Part 9 — Honest limitations

State these yourself rather than being caught by them:

- Loading is a short UMG transition after the map loads, not an asynchronous movie-player screen.
- Continue is intentionally disabled; there is no save system.
- Hit zones depend on physics-asset bone naming; unknown bones fall back to torso.
- Reaction animation coverage is limited to the existing Blueprint assets.
- This is a physics and reaction demonstration, not a finished game or a full combat system.
- External asset licence records are incomplete and must be verified before public redistribution.

---

## Quick reference — every number in one place

| Constant | Value | Where |
|---|---|---|
| Launch speed range | 6,000 – 9,200 cm/s | `PlayerCharacter.h` |
| Minimum charge | 0.3 s | `PlayerCharacter.h` |
| Maximum charge | 2.5 s | `PlayerCharacter.h` |
| Overdraw failure | 3.5 s | `PlayerCharacter.h` |
| Stone radius | 5.0 | `StoneProjectile.cpp` |
| Bounciness (restitution) | 0.32 | `StoneProjectile.cpp` |
| Friction | 0.28 | `StoneProjectile.cpp` |
| Bounce stop threshold | 250 | `StoneProjectile.cpp` |
| Max collisions | 2 | `StoneProjectile.h` |
| Stone lifetime | 8.0 s | `StoneProjectile.h` |
| Minimum damage speed | 800 | `StoneProjectile.h` |
| Simulation timestep | 0.025 s (40 Hz) | both |
| Angle guide length | 55 | `SlingshotAimGuideComponent.h` |
| Health | 100 | `ZombieCharacter.h` |
| Damage: torso / leg / arm | 30 / 20 / 15 | `ZombieCharacter.h` |
| Stagger duration | 2.0 s | `ZombieCharacter.h` |
| Crawling speed | 80 | `ZombieCharacter.h` |
| Death impulse | 4,500 | `ZombieCharacter.h` |
| Move: walk / sprint / crouch | 340 / 520 / 150 | `PlayerCharacter.h` |
