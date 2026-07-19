#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DOKPlayerController.generated.h"

class UInputMappingContext;

/** Installs the project's default Enhanced Input mapping contexts. */
UCLASS()
class DARKNESSOFKABUL_API ADOKPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADOKPlayerController();

protected:
	virtual void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly, Category = "DOK|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "DOK|Input")
	TObjectPtr<UInputMappingContext> MouseLookMappingContext;
};
