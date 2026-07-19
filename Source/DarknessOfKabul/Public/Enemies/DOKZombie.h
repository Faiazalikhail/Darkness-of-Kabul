#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DOKZombie.generated.h"

UENUM(BlueprintType)
enum class EDOKZombieState : uint8
{
	Idle,
	Walk,
	Stagger,
	Attack,
	Dead
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDOKZombieStateChanged, EDOKZombieState, NewState);

/** Intentionally small zombie target with walk, stagger, attack, and death states. */
UCLASS(Blueprintable)
class DARKNESSOFKABUL_API ADOKZombie : public ACharacter
{
	GENERATED_BODY()

public:
	ADOKZombie();

	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "DOK|Zombie")
	void SetTargetActor(AActor* NewTarget);

	UFUNCTION(BlueprintPure, Category = "DOK|Zombie")
	EDOKZombieState GetZombieState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "DOK|Zombie")
	float GetHealthPercent() const;

	UPROPERTY(BlueprintAssignable, Category = "DOK|Zombie")
	FDOKZombieStateChanged OnZombieStateChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Zombie", meta = (ClampMin = "1.0"))
	float MaximumHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Zombie", meta = (ClampMin = "1.0"))
	float WalkSpeed = 110.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Zombie", meta = (ClampMin = "1.0"))
	float AttackRange = 90.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Zombie", meta = (ClampMin = "0.0"))
	float AttackDamage = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Zombie", meta = (ClampMin = "0.1"))
	float AttackCooldown = 1.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Zombie", meta = (ClampMin = "0.0"))
	float StaggerDuration = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Zombie", meta = (ClampMin = "0.0"))
	float BodyHitStaggerThreshold = 1.0f;

private:
	void SetZombieState(EDOKZombieState NewState);
	void UpdateMovement();
	void TryAttack();
	void RecoverFromStagger();
	void Die();

	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

	FTimerHandle StaggerTimer;
	EDOKZombieState CurrentState = EDOKZombieState::Idle;
	float CurrentHealth = 100.0f;
	float LastAttackTime = -1000.0f;
};
