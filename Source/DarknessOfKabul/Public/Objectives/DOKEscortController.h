#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DOKEscortController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDOKRouteStageChanged, int32, NewStage, FText, ObjectiveText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDOKFlockDangerChanged, float, DangerPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDOKEscortCompleted);

/** Coordinates the linear flock route and its danger meter. */
UCLASS(Blueprintable)
class DARKNESSOFKABUL_API ADOKEscortController : public AActor
{
	GENERATED_BODY()

public:
	ADOKEscortController();

	UFUNCTION(BlueprintCallable, Category = "DOK|Escort")
	bool AdvanceStage();

	UFUNCTION(BlueprintCallable, Category = "DOK|Escort")
	void AddFlockDanger(float Amount);

	UFUNCTION(BlueprintCallable, Category = "DOK|Escort")
	void ReduceFlockDanger(float Amount);

	UFUNCTION(BlueprintCallable, Category = "DOK|Escort")
	void MarkFlockHome();

	UFUNCTION(BlueprintPure, Category = "DOK|Escort")
	int32 GetCurrentStage() const { return CurrentStage; }

	UFUNCTION(BlueprintPure, Category = "DOK|Escort")
	float GetDangerPercent() const;

	UFUNCTION(BlueprintPure, Category = "DOK|Escort")
	FText GetCurrentObjective() const;

	UPROPERTY(BlueprintAssignable, Category = "DOK|Escort")
	FDOKRouteStageChanged OnRouteStageChanged;

	UPROPERTY(BlueprintAssignable, Category = "DOK|Escort")
	FDOKFlockDangerChanged OnFlockDangerChanged;

	UPROPERTY(BlueprintAssignable, Category = "DOK|Escort")
	FDOKEscortCompleted OnEscortCompleted;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DOK|Escort")
	TArray<FText> StageObjectives;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DOK|Escort", meta = (ClampMin = "1.0"))
	float MaximumFlockDanger = 100.0f;

private:
	int32 CurrentStage = 0;
	float CurrentFlockDanger = 0.0f;
	bool bEscortComplete = false;
};
