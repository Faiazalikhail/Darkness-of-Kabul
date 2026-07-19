#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DOKHUDWidget.generated.h"

/** Blueprint-ready HUD contract. Visual layout is created in a Widget Blueprint. */
UCLASS(Abstract, Blueprintable)
class DARKNESSOFKABUL_API UDOKHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "DOK|HUD")
	void SetAmmo(int32 CurrentAmmo, int32 MaximumAmmo);

	UFUNCTION(BlueprintImplementableEvent, Category = "DOK|HUD")
	void SetDrawPercent(float DrawPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = "DOK|HUD")
	void SetHealthPercent(float HealthPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = "DOK|HUD")
	void SetFlockDangerPercent(float DangerPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = "DOK|HUD")
	void SetObjectiveText(const FText& ObjectiveText);

	UFUNCTION(BlueprintImplementableEvent, Category = "DOK|HUD")
	void ShowEndState(bool bWon, const FText& Message);
};
