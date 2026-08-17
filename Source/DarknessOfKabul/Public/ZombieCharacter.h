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

	/**
	 * Wakes this zombie and sends it after the player. Used both by direct
	 * sight and by the disturbance that spreads from a struck zombie.
	 */
	UFUNCTION(BlueprintCallable, Category = "Zombie|AI")
	void AlertToPlayer(APawn* PlayerPawn);

	/** True once this zombie has seen or been alerted to the player. */
	UFUNCTION(BlueprintPure, Category = "Zombie|AI")
	bool IsAlerted() const { return bAlerted; }

	/** Drives chase and attack animation selection in the Animation Blueprint. */
	UFUNCTION(BlueprintPure, Category = "Zombie|AI")
	bool IsChasing() const;

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

	/** How far this zombie can notice the player in a straight line. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie|AI", meta = (ClampMin = "0"))
	float SightRange = 2600.0f;

	/** Half-angle of the forward vision cone, in degrees. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie|AI", meta = (ClampMin = "1", ClampMax = "180"))
	float SightHalfAngleDegrees = 75.0f;

	/** Movement speed once alerted. Deliberately faster than the idle walk. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie|AI", meta = (ClampMin = "0"))
	float ChaseMaxWalkSpeed = 520.0f;

	/** Distance at which the zombie stops closing and starts swinging. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie|AI", meta = (ClampMin = "1"))
	float AttackRange = 175.0f;

	/** Damage applied to the player on each connected swing. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie|AI", meta = (ClampMin = "0"))
	float AttackDamage = 25.0f;

	/** Seconds between swings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie|AI", meta = (ClampMin = "0.1"))
	float AttackInterval = 1.1f;

	/**
	 * Radius of the disturbance raised when this zombie is struck. Only zombies
	 * inside this radius join the attack, so a hit pulls the local group rather
	 * than the whole level.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie|AI", meta = (ClampMin = "0"))
	float DisturbanceRadius = 1800.0f;

	/**
	 * Distance at which an alerted zombie gives up and returns to unaware.
	 * 12000 units is 120 metres, so outrunning the group genuinely escapes it.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie|AI", meta = (ClampMin = "0"))
	float LoseInterestDistance = 12000.0f;

	/** Seconds between AI sense/decide passes. Kept off the render frame. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie|AI", meta = (ClampMin = "0.02"))
	float SenseInterval = 0.2f;

	/** Use this event to play the attack animation on a swing. */
	UFUNCTION(
		BlueprintImplementableEvent,
		Category = "Zombie|AI",
		meta = (DisplayName = "On Zombie Attack")
	)
	void BP_OnZombieAttack(FVector PlayerLocation);

	/** Use this event for a growl or alert cue when the zombie wakes. */
	UFUNCTION(
		BlueprintImplementableEvent,
		Category = "Zombie|AI",
		meta = (DisplayName = "On Zombie Alerted")
	)
	void BP_OnZombieAlerted();

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

	/** One sense-and-decide pass. Driven by a timer, not by Tick. */
	void UpdateAI();
	bool CanSeePlayer(const APawn* PlayerPawn) const;
	/** Wakes every living zombie within DisturbanceRadius of this one. */
	void RaiseDisturbance(APawn* PlayerPawn);
	/** Applies the walk speed matching the current state and alert status. */
	void RefreshMovementSpeed();
	APawn* FindPlayerPawn() const;

	FTimerHandle AISenseTimerHandle;
	bool bAlerted = false;
	float LastAttackTime = -1000.0f;
	TWeakObjectPtr<APawn> ChaseTarget;

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
