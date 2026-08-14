#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZombieCharacter.generated.h"

/** Body region resolved from the Physics Asset bone struck by a stone. */
UENUM(BlueprintType)
enum class EZombieHitZone : uint8
{
	Head UMETA(DisplayName = "Head / Neck"),
	Torso UMETA(DisplayName = "Torso"),
	Arm UMETA(DisplayName = "Arm"),
	Leg UMETA(DisplayName = "Leg")
};

/** Physical reaction state, kept separate from the future AI state. */
UENUM(BlueprintType)
enum class EZombiePhysicalState : uint8
{
	Standing UMETA(DisplayName = "Standing"),
	Staggered UMETA(DisplayName = "Staggered"),
	Crawling UMETA(DisplayName = "Crawling"),
	Dead UMETA(DisplayName = "Dead")
};

/**
 * Prototype zombie target with Physics Asset bone-hit reactions.
 * AI is deliberately separate and can be layered on later.
 */
UCLASS(Blueprintable)
class DARKNESSOFKABUL_API AZombieCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AZombieCharacter();

	/** Applies one stone impact using the exact Physics Asset bone that was hit. */
	UFUNCTION(BlueprintCallable, Category = "Zombie|Damage")
	void ReceiveStoneImpact(
		const FHitResult& Hit,
		const FVector& ImpactVelocity
	);

	/** Restores this placed target for another portfolio demonstration pass. */
	UFUNCTION(BlueprintCallable, Category = "Zombie|Reset")
	void ResetReactionState();

protected:
	virtual void BeginPlay() override;

	/** Maximum and starting health. Head hits bypass this value. */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Zombie|Damage",
		meta = (ClampMin = "1.0")
	)
	float MaxHealth = 100.0f;

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Zombie|Damage"
	)
	float CurrentHealth = 100.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Zombie|Damage",
		meta = (ClampMin = "0.0")
	)
	float TorsoDamage = 30.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Zombie|Damage",
		meta = (ClampMin = "0.0")
	)
	float ArmDamage = 15.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Zombie|Damage",
		meta = (ClampMin = "0.0")
	)
	float LegDamage = 20.0f;

	/** How long a living zombie is unable to move after a torso hit. */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Zombie|Reaction",
		meta = (ClampMin = "0.0")
	)
	float TorsoStaggerDuration = 2.0f;

	/** Movement speed retained after a leg has been broken. */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Zombie|Reaction",
		meta = (ClampMin = "0.0")
	)
	float CrawlingMaxSpeed = 80.0f;

	/** Ragdoll push applied in the stone's travel direction on death. */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Zombie|Reaction",
		meta = (ClampMin = "0.0")
	)
	float DeathImpulse = 4500.0f;

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Zombie|State"
	)
	EZombiePhysicalState PhysicalState =
		EZombiePhysicalState::Standing;

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Zombie|State"
	)
	EZombieHitZone LastHitZone = EZombieHitZone::Torso;

	/** Optional editor/development diagnostic; never shown in Shipping. */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Zombie|Debug"
	)
	bool bShowHitDebug = false;

	/** Use this event to select a hit animation and spawn impact effects. */
	UFUNCTION(
		BlueprintImplementableEvent,
		Category = "Zombie|Reaction",
		meta = (DisplayName = "On Zombie Hit")
	)
	void BP_OnZombieHit(
		EZombieHitZone HitZone,
		FName BoneName,
		float Damage,
		FVector ImpactPoint
	);

	/** Use this event to swap standing, stagger, and crawling animation. */
	UFUNCTION(
		BlueprintImplementableEvent,
		Category = "Zombie|Reaction",
		meta = (DisplayName = "On Physical State Changed")
	)
	void BP_OnPhysicalStateChanged(
		EZombiePhysicalState OldState,
		EZombiePhysicalState NewState
	);

	/** Called immediately before the skeletal mesh becomes a ragdoll. */
	UFUNCTION(
		BlueprintImplementableEvent,
		Category = "Zombie|Reaction",
		meta = (DisplayName = "On Zombie Death")
	)
	void BP_OnZombieDeath(
		EZombieHitZone FatalZone,
		FVector ImpactPoint
	);

private:
	EZombieHitZone ClassifyHitBone(FName BoneName) const;
	void BeginStagger();
	void EndStagger();
	void EnterCrawling();
	void Die(
		EZombieHitZone FatalZone,
		const FHitResult& Hit,
		const FVector& ImpactVelocity
	);
	void SetPhysicalState(EZombiePhysicalState NewState);
	void ShowHitDebug(EZombieHitZone HitZone, FName BoneName) const;

	FTimerHandle StaggerTimerHandle;
	EZombiePhysicalState StateBeforeStagger =
		EZombiePhysicalState::Standing;
	float StandingMaxWalkSpeed = 300.0f;
	FTransform InitialActorTransform;
	FTransform InitialMeshRelativeTransform;
	FName InitialCapsuleCollisionProfile;
	FName InitialMeshCollisionProfile;
	ECollisionEnabled::Type InitialCapsuleCollisionEnabled =
		ECollisionEnabled::QueryAndPhysics;
	ECollisionEnabled::Type InitialMeshCollisionEnabled =
		ECollisionEnabled::QueryAndPhysics;
};
