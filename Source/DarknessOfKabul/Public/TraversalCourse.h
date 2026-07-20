#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TraversalCourse.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

/**
 * Editable milestone-one blockout made from ordinary Unreal components.
 * This is level geometry only: it contains no targets, damage, or gameplay rules.
 */
UCLASS(Blueprintable)
class DARKNESSOFKABUL_API ATraversalCourse : public AActor
{
	GENERATED_BODY()

public:
	ATraversalCourse();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone One|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone One|Course")
	TObjectPtr<UStaticMeshComponent> JumpHurdle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone One|Course")
	TObjectPtr<UStaticMeshComponent> CrouchTunnelRoof;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone One|Course")
	TObjectPtr<UStaticMeshComponent> CrouchTunnelLeftWall;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone One|Course")
	TObjectPtr<UStaticMeshComponent> CrouchTunnelRightWall;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone One|Course")
	TObjectPtr<UStaticMeshComponent> LowClimbBlock;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone One|Course")
	TObjectPtr<UStaticMeshComponent> MediumClimbBlock;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone One|Course")
	TObjectPtr<UStaticMeshComponent> HighClimbBlock;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone One|Guide")
	TObjectPtr<UTextRenderComponent> GuideText;

	// Future level work belongs in normal Unreal objects or Blueprint children:
	// - visual materials and environment art;
	// - checkpoints and objectives;
	// - projectile-practice props;
	// - enemies and encounter scripting.
};
