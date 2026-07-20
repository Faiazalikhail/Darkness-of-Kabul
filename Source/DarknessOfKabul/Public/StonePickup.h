#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StonePickup.generated.h"

/** Blueprint base reserved for a future stone-ammunition pickup. */
UCLASS(Blueprintable)
class DARKNESSOFKABUL_API AStonePickup : public AActor
{
	GENERATED_BODY()

	// Collection and ammunition behavior will be added only when needed.
};
