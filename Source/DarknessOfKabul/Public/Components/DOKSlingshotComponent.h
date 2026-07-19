#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DOKSlingshotComponent.generated.h"

class ADOKStoneProjectile;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDOKAmmoChanged, int32, NewAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDOKDrawChanged, float, DrawPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDOKShotReleased, float, LaunchSpeed);

/** Owns slingshot draw state, ammunition, spawning, and trajectory preview. */
UCLASS(ClassGroup = (DOK), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class DARKNESSOFKABUL_API UDOKSlingshotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDOKSlingshotComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "DOK|Slingshot")
	bool StartDrawing();

	UFUNCTION(BlueprintCallable, Category = "DOK|Slingshot")
	void CancelDrawing();

	UFUNCTION(BlueprintCallable, Category = "DOK|Slingshot")
	bool ReleaseShot(const FVector& AimDirection);

	UFUNCTION(BlueprintPure, Category = "DOK|Slingshot")
	float GetDrawPercent() const;

	UFUNCTION(BlueprintPure, Category = "DOK|Slingshot")
	float CalculateLaunchSpeed() const;

	UFUNCTION(BlueprintPure, Category = "DOK|Slingshot")
	bool IsDrawing() const { return bIsDrawing; }

	UFUNCTION(BlueprintPure, Category = "DOK|Slingshot")
	int32 GetAmmo() const { return CurrentAmmo; }

	UFUNCTION(BlueprintPure, Category = "DOK|Slingshot")
	int32 GetMaximumAmmo() const { return MaximumAmmo; }

	UFUNCTION(BlueprintCallable, Category = "DOK|Slingshot")
	int32 AddAmmo(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "DOK|Slingshot")
	void ToggleTrajectoryGuide();

	UFUNCTION(BlueprintPure, Category = "DOK|Slingshot")
	TArray<FVector> BuildTrajectoryPreview(const FVector& AimDirection) const;

	UPROPERTY(BlueprintAssignable, Category = "DOK|Slingshot")
	FDOKAmmoChanged OnAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "DOK|Slingshot")
	FDOKDrawChanged OnDrawChanged;

	UPROPERTY(BlueprintAssignable, Category = "DOK|Slingshot")
	FDOKShotReleased OnShotReleased;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Slingshot|Physics", meta = (ClampMin = "0.0"))
	float MinimumLaunchSpeed = 2500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Slingshot|Physics", meta = (ClampMin = "0.0"))
	float MaximumLaunchSpeed = 6000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Slingshot|Physics", meta = (ClampMin = "0.01"))
	float MaximumDrawTime = 1.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Slingshot|Physics")
	FVector Gravity = FVector(0.0, 0.0, -980.0);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Slingshot|Rules", meta = (ClampMin = "0"))
	int32 StartingAmmo = 8;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Slingshot|Rules", meta = (ClampMin = "1"))
	int32 MaximumAmmo = 15;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Slingshot|Rules", meta = (ClampMin = "0.0"))
	float FireCooldown = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Slingshot|Spawn")
	TSubclassOf<ADOKStoneProjectile> StoneClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Slingshot|Spawn", meta = (ClampMin = "0.0"))
	float SpawnDistanceFromCamera = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Slingshot|Preview", meta = (ClampMin = "0.1"))
	float PreviewTime = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Slingshot|Preview", meta = (ClampMin = "0.01"))
	float PreviewTimeStep = 0.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DOK|Slingshot|Preview")
	bool bTrajectoryGuideEnabled = true;

private:
	FVector GetLaunchPosition(const FVector& AimDirection) const;
	void DrawTrajectoryPreview() const;

	int32 CurrentAmmo = 0;
	float DrawStartTime = 0.0f;
	float LastFireTime = -1000.0f;
	bool bIsDrawing = false;
};
