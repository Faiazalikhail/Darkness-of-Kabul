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

public:
	AKabulGameMode();

	void NotifyZombieDefeated(class AZombieCharacter* Zombie);
	void NotifyWobbleTargetHit(AActor* WobbleTarget);
	void ResetPrototypeObjectives();

	int32 GetZombieObjectiveCount() const { return ZombieObjectives.Num(); }
	int32 GetDefeatedZombieCount() const { return DefeatedZombies.Num(); }
	int32 GetWobbleObjectiveCount() const { return WobbleObjectives.Num(); }
	int32 GetHitWobbleCount() const { return HitWobbleTargets.Num(); }

protected:
	virtual void BeginPlay() override;

private:
	void CheckForPrototypeCompletion();

	TSet<TWeakObjectPtr<AZombieCharacter>> ZombieObjectives;
	TSet<TWeakObjectPtr<AZombieCharacter>> DefeatedZombies;
	TSet<TWeakObjectPtr<AActor>> WobbleObjectives;
	TSet<TWeakObjectPtr<AActor>> HitWobbleTargets;
	TMap<TWeakObjectPtr<AActor>, FTransform> WobbleStartTransforms;
	bool bCompletionShown = false;
};
