#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DOKKinematicsLibrary.generated.h"

/** Reusable analytical projectile equations shared by the preview and real stone. */
UCLASS()
class DARKNESSOFKABUL_API UDOKKinematicsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DOK|Physics")
	static FVector PositionAtTime(
		const FVector& StartPosition,
		const FVector& InitialVelocity,
		const FVector& Acceleration,
		float Time);

	UFUNCTION(BlueprintPure, Category = "DOK|Physics")
	static FVector VelocityAtTime(
		const FVector& InitialVelocity,
		const FVector& Acceleration,
		float Time);

	UFUNCTION(BlueprintPure, Category = "DOK|Physics")
	static TArray<FVector> BuildTrajectoryPoints(
		const FVector& StartPosition,
		const FVector& InitialVelocity,
		const FVector& Acceleration,
		float MaxTime = 2.0f,
		float TimeStep = 0.05f);
};
