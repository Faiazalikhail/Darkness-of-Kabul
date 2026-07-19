#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DOKStonePickup.generated.h"

class ADOKCharacter;
class USphereComponent;
class UStaticMeshComponent;

/** Interactable pickup that adds stones to the player's slingshot. */
UCLASS(Blueprintable)
class DARKNESSOFKABUL_API ADOKStonePickup : public AActor
{
	GENERATED_BODY()

public:
	ADOKStonePickup();

	UFUNCTION(BlueprintCallable, Category = "DOK|Pickup")
	bool Collect(ADOKCharacter* Character);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DOK|Components")
	TObjectPtr<USphereComponent> InteractionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DOK|Components")
	TObjectPtr<UStaticMeshComponent> PickupMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DOK|Pickup", meta = (ClampMin = "1"))
	int32 StoneAmount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DOK|Pickup")
	bool bAutoCollectOnOverlap = false;
};
