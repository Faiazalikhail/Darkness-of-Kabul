#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SlingshotTrajectoryTypes.h"
#include "SlingshotAimGuideComponent.generated.h"

/**
 * Produces the collision-aware path used by the presentation HUD.
 * Rendering stays in the HUD so the Shipping build does not depend on
 * temporary DrawDebug output.
 */
UCLASS(ClassGroup = (Weapon), meta = (BlueprintSpawnableComponent))
class DARKNESSOFKABUL_API USlingshotAimGuideComponent
	: public UActorComponent
{
	GENERATED_BODY()

public:
	USlingshotAimGuideComponent();

	bool PredictTrajectory(
		const FVector& StartLocation,
		const FVector& LaunchVelocity,
		float ProjectileRadius,
		float ProjectileGravityScale,
		float Bounciness,
		float Friction,
		int32 MaximumCollisionCount,
		float MaximumSimulationTime,
		const AActor* ActorToIgnore,
		FSlingshotTrajectoryPrediction& OutPrediction
	) const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Aim Guide", meta = (ClampMin = "0.1"))
	float SimulationFrequency = 40.0f;

	/** Length of the two small line pieces that communicate a bounce angle. */
	UPROPERTY(EditDefaultsOnly, Category = "Aim Guide", meta = (ClampMin = "5.0"))
	float AngleGuideLength = 55.0f;
};
