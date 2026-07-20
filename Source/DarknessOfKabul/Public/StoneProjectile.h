#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StoneProjectile.generated.h"

/**
 * Blueprint base for the slingshot stone.
 * Components, motion, collision sweeps, and hit reactions are deferred.
 */
UCLASS(Blueprintable)
class DARKNESSOFKABUL_API AStoneProjectile : public AActor
{
	GENERATED_BODY()
};
