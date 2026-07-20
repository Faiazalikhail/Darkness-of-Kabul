#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
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

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY()
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> MouseLookMappingContext;

	UPROPERTY()
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY()
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY()
	TObjectPtr<UInputAction> MouseLookAction;

	UPROPERTY()
	TObjectPtr<UInputAction> JumpAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartJump();
	void EndJump();
};
