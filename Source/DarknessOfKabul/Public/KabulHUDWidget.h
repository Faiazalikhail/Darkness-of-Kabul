#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KabulHUDWidget.generated.h"

/** Optional Blueprint base for the future crosshair and shooting feedback. */
UCLASS(Abstract, Blueprintable)
class DARKNESSOFKABUL_API UKabulHUDWidget : public UUserWidget
{
	GENERATED_BODY()

	// UI events will be added only after the underlying mechanic exists.
};
