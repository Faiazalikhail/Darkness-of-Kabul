#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SlingshotTrajectoryTypes.h"
#include "PlayerCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class USkeletalMeshComponent;
class USlingshotAimGuideComponent;
class UKabulPrototypeUI;
class AStoneProjectile;
struct FInputActionValue;

/**
 * Blueprint base for the first-person player.
 *
 * Owns first-person locomotion: walk, look, jump, sprint, crouch, and a
 * simple collision-safe ledge climb. Weapon and combat logic live elsewhere.
 */
UCLASS(Blueprintable)
class DARKNESSOFKABUL_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	// Returns the first-person camera component
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** True while the player is holding the slingshot in its aiming position. */
	UFUNCTION(BlueprintPure, Category = "Weapon|Slingshot")
	bool IsSlingshotAiming() const { return bIsAiming; }

	/** Current pull amount from 0 (not pulled) to 1 (fully pulled). */
	UFUNCTION(BlueprintPure, Category = "Weapon|Slingshot")
	float GetSlingshotPullAmount() const;

	/** True once releasing the current pull will launch a stone. */
	UFUNCTION(BlueprintPure, Category = "Weapon|Slingshot")
	bool IsSlingshotShotReady() const;

	/** Up to two predicted impacts rendered as markers and short angle pieces. */
	bool GetSlingshotTrajectory(
		FSlingshotTrajectoryPrediction& OutPrediction
	) const;

	/** Remaining time for the short reset confirmation shown by the HUD. */
	float GetResetFeedbackTimeRemaining() const;

	/** Settings-menu controls. */
	void SetLookSensitivity(float NewSensitivity);
	void SetFirstPersonFieldOfView(float NewFieldOfView);
	float GetLookSensitivity() const { return LookSensitivity; }

	/** Restarts the player, placed targets, objectives, and stones. */
	void RestartPrototype();
	void TogglePrototypePause();
	void ShowPrototypeCompletion();

	/** Multiplier reserved for future footsteps or AI hearing. Crouching is quietest. */
	UFUNCTION(BlueprintPure, Category = "Movement|Noise")
	float GetMovementNoiseMultiplier() const { return MovementNoiseMultiplier; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

private:
	/* --- COMPONENTS --- */

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	/** First person mesh (arms), seen only by self */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh1P;

	/** Draws the target dot and curved stone-flight preview. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Slingshot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USlingshotAimGuideComponent> SlingshotAimGuide;


	/* --- SLINGSHOT --- */



	/** Aiming **/

	/** True while the player is holding the right mouse button. */
	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Weapon|Slingshot",
		meta = (AllowPrivateAccess = "true")
	)
	bool bIsAiming = false;

	void StartAiming();
	void StopAiming();




	/** Shooting **/
/** Projectile Blueprint created when the player fires. */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Weapon|Slingshot",
		meta = (AllowPrivateAccess = "true")
	)
	TSubclassOf<AStoneProjectile> StoneProjectileClass;

	/** Minimum speed produced by a quick click. */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Weapon|Slingshot",
		meta = (AllowPrivateAccess = "true", ClampMin = "0")
	)
	float MinimumLaunchSpeed = 6000.0f;

	/** Maximum speed produced by a fully charged shot. */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Weapon|Slingshot",
		meta = (AllowPrivateAccess = "true", ClampMin = "0")
	)
	float MaximumLaunchSpeed = 9200.0f;

	/** Minimum pull time required before a stone can be fired. */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Weapon|Slingshot",
		meta = (AllowPrivateAccess = "true", ClampMin = "0")
	)
	float MinimumChargeTime = 0.3f;

	/** Time required to reach maximum launch speed. */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Weapon|Slingshot",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.01")
	)
	float MaximumChargeTime = 2.5f;

	/** Old overdraw mechanic: a held pull fails and shakes the camera here. */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Weapon|Slingshot",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.1")
	)
	float MaximumDrawDuration = 3.5f;

	/** Distance used to establish the camera-aligned aim point. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Slingshot", meta = (ClampMin = "100.0"))
	float AimTargetDistance = 8000.0f;

	/** Offset from the camera to the pulled stone and trajectory origin. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Slingshot")
	FVector LaunchOffset = FVector(75.0f, -45.0f, -24.0f);

	float ChargeStartTime = 0.0f;
	bool bChargingStone = false;

	void StartChargingStone();
	void ReleaseChargedStone();
	void FireStone(float LaunchSpeed);
	float GetSlingshotHeldTime() const;
	void UpdateSlingshotAim();
	bool ComputeSlingshotAim(
		FVector& OutLaunchLocation,
		FVector& OutLaunchDirection
	) const;
	void ResetPrototype();
	void InitializePrototypeUI();

	float ResetFeedbackEndTime = 0.0f;
	float LookSensitivity = 1.0f;
	FTransform InitialPlayerTransform;
	FRotator InitialControlRotation = FRotator::ZeroRotator;

	UPROPERTY(Transient)
	TObjectPtr<UKabulPrototypeUI> PrototypeUI;


	/* --- LOCOMOTION TUNING --- */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Speed", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	float WalkSpeed = 340.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Speed", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	float SprintSpeed = 520.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Speed", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	float CrouchWalkSpeed = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Noise", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	float WalkNoiseMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Noise", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	float SprintNoiseMultiplier = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Noise", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	float CrouchNoiseMultiplier = 0.25f;

	/** Horizontal distance used to find a ledge in front of the player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Climb", meta = (AllowPrivateAccess = "true", ClampMin = "10"))
	float ClimbReach = 90.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Climb", meta = (AllowPrivateAccess = "true", ClampMin = "10"))
	float MinimumClimbHeight = 40.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Climb", meta = (AllowPrivateAccess = "true", ClampMin = "20"))
	float MaximumClimbHeight = 140.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Climb", meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	float ClimbLandingInset = 45.0f;

	/** A normal same-height jump stays below this; short drops trigger feedback. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Landing", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	float LandingShakeMinimumSpeed = 480.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Landing", meta = (AllowPrivateAccess = "true", ClampMin = "0.05"))
	float LandingShakeDuration = 0.32f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Landing", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	float MaximumLandingCameraDrop = 6.0f;

	float MovementNoiseMultiplier = 1.0f;
	bool bSprintHeld = false;
	FVector CameraRestingLocation = FVector::ZeroVector;
	FRotator CameraRestingRotation = FRotator::ZeroRotator;
	float LandingShakeElapsed = 0.0f;
	float LandingShakeStrength = 0.0f;


	/* --- INPUT ASSETS --- */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Contexts", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Contexts", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> MouseLookMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MouseLookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;


	/* --- INPUT FUNCTIONS --- */

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartJump();
	void EndJump();
	void StartSprint();
	void StopSprint();
	void StartCrouch();
	void StopCrouch();
	void TogglePauseMenu();
	void UpdateLocomotionState();
	bool TryClimb();
};
