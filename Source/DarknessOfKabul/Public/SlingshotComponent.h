#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SlingshotComponent.generated.h"

/**
 * Owns the future aim, draw, launch-speed, ammunition, and shot-release state.
 * Keeping this separate from the character lets the main mechanic evolve
 * without turning the player class into one large file.
 */
UCLASS(ClassGroup = (Gameplay), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class DARKNESSOFKABUL_API USlingshotComponent : public UActorComponent
{
	GENERATED_BODY()

	// Aim and shooting functions will be introduced one mechanic at a time.
};
