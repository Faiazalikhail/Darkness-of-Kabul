#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class USkeletalMeshComponent;
struct FInputActionValue;

/**
 * Blueprint base for the first-person player.
 *
 * Milestone one owns locomotion only: walk, look, jump, sprint, crouch, and
 * a simple collision-safe ledge climb. Weapon and combat work stay deferred.
 */
UCLASS(Blueprintable)
class DARKNESSOFKABUL_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	// Returns the first-person camera component
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** Multiplier reserved for future footsteps or AI hearing. Crouching is quietest. */
	UFUNCTION(BlueprintPure, Category = "Movement|Noise")
	float GetMovementNoiseMultiplier() const { return MovementNoiseMultiplier; }

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;
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


	/* --- LOCOMOTION TUNING --- */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Speed", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	float WalkSpeed = 400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Speed", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	float SprintSpeed = 650.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Speed", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	float CrouchWalkSpeed = 180.0f;

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

	float MovementNoiseMultiplier = 1.0f;
	bool bSprintHeld = false;


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
	void UpdateLocomotionState();
	bool TryClimb();
};
