#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DOKCharacter.generated.h"

class UCameraComponent;
class UDOKSlingshotComponent;
class UInputAction;
class USkeletalMeshComponent;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDOKHealthChanged, float, NewHealthPercent);

/** Playable first-person shepherd character. */
UCLASS(Blueprintable)
class DARKNESSOFKABUL_API ADOKCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ADOKCharacter();

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure, Category = "DOK|Character")
	UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

	UFUNCTION(BlueprintPure, Category = "DOK|Character")
	UDOKSlingshotComponent* GetSlingshot() const { return SlingshotComponent; }

	UFUNCTION(BlueprintPure, Category = "DOK|Character")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "DOK|Character")
	bool IsAiming() const { return bIsAiming; }

	UPROPERTY(BlueprintAssignable, Category = "DOK|Character")
	FDOKHealthChanged OnHealthChanged;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DOK|Components")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DOK|Components")
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DOK|Components")
	TObjectPtr<UDOKSlingshotComponent> SlingshotComponent;

	UPROPERTY(EditDefaultsOnly, Category = "DOK|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "DOK|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "DOK|Input")
	TObjectPtr<UInputAction> MouseLookAction;

	UPROPERTY(EditDefaultsOnly, Category = "DOK|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Movement")
	float WalkSpeed = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Movement")
	float SprintSpeed = 750.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Movement")
	float AimSpeed = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Camera")
	float NormalFieldOfView = 90.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Camera")
	float AimFieldOfView = 75.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Health", meta = (ClampMin = "1.0"))
	float MaximumHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Interaction", meta = (ClampMin = "1.0"))
	float InteractionDistance = 250.0f;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartJump();
	void StopJump();
	void StartAim();
	void StopAim();
	void StartFire();
	void ReleaseFire();
	void StartSprint();
	void StopSprint();
	void Interact();
	void ToggleTrajectoryGuide();
	void RestartLevel();
	void UpdateMovementSpeed();

	float CurrentHealth = 100.0f;
	bool bIsAiming = false;
	bool bIsSprinting = false;
};
