#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KabulPlayerController.generated.h"

/** Player-controller extension point for input contexts and player-only UI. */
UCLASS(Blueprintable)
class DARKNESSOFKABUL_API AKabulPlayerController : public APlayerController
{
	GENERATED_BODY()

	// Enhanced Input mapping contexts will be installed in a later pass.
};
