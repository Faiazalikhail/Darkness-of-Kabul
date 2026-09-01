#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

/**
 * One health pool, shared by every actor that can be damaged.
 *
 * The player and the zombies previously each carried their own copy of the
 * same arithmetic. The rules live here once; each owner still decides what
 * running out of health means for it, because a ragdoll and a game over need
 * different context than a health value can carry.
 */
UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class DARKNESSOFKABUL_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	/** Sets the pool size and fills it. Call from the owner's BeginPlay. */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void InitializeHealth(float InMaxHealth);

	/** Subtracts damage and returns how much was actually removed. */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float ApplyDamage(float Amount);

	/** Empties the pool in one step, for hits that are lethal by rule. */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float ApplyLethalDamage();

	/** Refills the pool to its current maximum. */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ResetHealth();

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }

	/** Remaining health from 0 to 1, for bars and readouts. */
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDepleted() const { return CurrentHealth <= 0.0f; }

private:
	float MaxHealth = 100.0f;
	float CurrentHealth = 100.0f;
};
