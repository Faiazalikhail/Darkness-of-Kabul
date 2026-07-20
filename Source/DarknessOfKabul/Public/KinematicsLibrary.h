#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "KinematicsLibrary.generated.h"

/**
 * Shared home for the projectile equations used by both the real stone and
 * the trajectory preview. The equations will be added during the physics pass.
 */
UCLASS()
class DARKNESSOFKABUL_API UKinematicsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
};
