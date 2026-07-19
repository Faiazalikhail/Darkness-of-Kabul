#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DOKGameMode.generated.h"

class UDOKHUDWidget;

UENUM(BlueprintType)
enum class EDOKGameState : uint8
{
	Playing,
	Won,
	Lost
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDOKGameStateChanged, EDOKGameState, NewState, FText, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDOKObjectiveChanged, FText, NewObjective);

/** Owns the level-wide objective, win, loss, and restart rules. */
UCLASS(Blueprintable)
class DARKNESSOFKABUL_API ADOKGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADOKGameMode();

	UFUNCTION(BlueprintCallable, Category = "DOK|Game")
	void WinGame();

	UFUNCTION(BlueprintCallable, Category = "DOK|Game")
	void LoseGame(const FString& Reason);

	UFUNCTION(BlueprintCallable, Category = "DOK|Game")
	void RestartCurrentLevel();

	UFUNCTION(BlueprintCallable, Category = "DOK|Game")
	void SetObjective(const FText& NewObjective);

	UFUNCTION(BlueprintPure, Category = "DOK|Game")
	EDOKGameState GetDOKGameState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "DOK|Game")
	FText GetObjective() const { return CurrentObjective; }

	UPROPERTY(BlueprintAssignable, Category = "DOK|Game")
	FDOKGameStateChanged OnGameStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "DOK|Game")
	FDOKObjectiveChanged OnObjectiveChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|UI")
	TSubclassOf<UDOKHUDWidget> HUDWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UDOKHUDWidget> HUDWidget;

	EDOKGameState CurrentState = EDOKGameState::Playing;
	FText CurrentObjective;
};
