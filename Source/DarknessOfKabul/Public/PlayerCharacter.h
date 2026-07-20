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
 * Planned implementation order:
 * 1. Camera and first-person mesh components.
 * 2. Move, look, jump, sprint, crouch, and slide input.
 * 3. Aim and shoot input forwarded to USlingshotComponent.
 *
 * Gameplay code is intentionally deferred so each part can be built and
 * explained separately.
 */
UCLASS(Blueprintable)
class DARKNESSOFKABUL_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	// Returns the first-person camera component
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;

private:
	/* --- COMPONENTS --- */

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	/** First person mesh (arms), seen only by self */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh1P;


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
};
