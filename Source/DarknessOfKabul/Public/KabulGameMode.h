#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "KabulGameMode.generated.h"

/**
 * Blueprint-configurable game mode for selecting the pawn and controller.
 * Level rules and story objectives do not belong in this foundation pass.
 */
UCLASS(Blueprintable)
class DARKNESSOFKABUL_API AKabulGameMode : public AGameModeBase
{
	GENERATED_BODY()
};
