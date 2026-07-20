#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZombieCharacter.generated.h"

/**
 * Blueprint base for the same target actor that will later become an enemy.
 * For the first hunting-ground map it can remain stationary while aim and
 * projectile hits are developed.
 */
UCLASS(Blueprintable)
class DARKNESSOFKABUL_API AZombieCharacter : public ACharacter
{
	GENERATED_BODY()

	// Health, reactions, and AI are intentionally deferred.
};
