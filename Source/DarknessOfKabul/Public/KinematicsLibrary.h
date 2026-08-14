#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "KinematicsLibrary.generated.h"

/** Shared charge calculations used by both the stone and its HUD preview. */
UCLASS()
class DARKNESSOFKABUL_API UKinematicsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Normalized usable charge after the minimum pull time has elapsed. */
	UFUNCTION(BlueprintPure, Category = "Slingshot|Physics")
	static float CalculateChargeAmount(
		float HeldTime,
		float MinimumChargeTime,
		float MaximumChargeTime
	);

	/** Launch speed for a valid shot, clamped to the configured speed range. */
	UFUNCTION(BlueprintPure, Category = "Slingshot|Physics")
	static float CalculateLaunchSpeed(
		float HeldTime,
		float MinimumChargeTime,
		float MaximumChargeTime,
		float MinimumLaunchSpeed,
		float MaximumLaunchSpeed
	);
};
