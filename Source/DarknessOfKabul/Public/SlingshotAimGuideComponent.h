#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SlingshotAimGuideComponent.generated.h"

class UCameraComponent;

/**
 * Draws the two visual parts of the slingshot aiming guide:
 *
 * 1. A small red dot at the camera's straight, gravity-free target.
 * 2. A lower curved path showing how the stone is expected to fly.
 *
 * This component only draws information. It does not move the camera,
 * change the player mesh, or launch the projectile.
 */
UCLASS(ClassGroup = (Weapon), meta = (BlueprintSpawnableComponent))
class DARKNESSOFKABUL_API USlingshotAimGuideComponent
	: public UActorComponent
{
	GENERATED_BODY()

public:
	USlingshotAimGuideComponent();

	void DrawGuide(
		const UCameraComponent* Camera,
		bool bIsAiming,
		bool bIsCharging,
		float HeldTime,
		float MinimumChargeTime,
		float MaximumChargeTime,
		float MinimumLaunchSpeed,
		float MaximumLaunchSpeed
	) const;

private:
	/** Maximum distance checked by the gravity-free red target dot. */
	UPROPERTY(EditDefaultsOnly, Category = "Aim Guide", meta = (ClampMin = "100"))
	float TargetDistance = 8000.0f;

	/** Distance in front of the camera where the curved guide begins. */
	UPROPERTY(EditDefaultsOnly, Category = "Aim Guide", meta = (ClampMin = "0"))
	float TrajectoryStartDistance = 60.0f;

	/** Keeps the curved guide visibly below the red target dot. */
	UPROPERTY(EditDefaultsOnly, Category = "Aim Guide", meta = (ClampMin = "0"))
	float TrajectoryVerticalOffset = 14.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Aim Guide", meta = (ClampMin = "2"))
	int32 TrajectoryPointCount = 18;

	UPROPERTY(EditDefaultsOnly, Category = "Aim Guide", meta = (ClampMin = "0.001"))
	float TrajectoryTimeStep = 0.009f;
};
