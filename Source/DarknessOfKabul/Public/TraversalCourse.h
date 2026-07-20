#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TraversalCourse.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

/**
 * Editable traversal blockout made from ordinary Unreal components.
 * This is level geometry only: it contains no targets, damage, or gameplay rules.
 */
UCLASS(Blueprintable)
class DARKNESSOFKABUL_API ATraversalCourse : public AActor
{
	GENERATED_BODY()

public:
	ATraversalCourse();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Course")
	TObjectPtr<UStaticMeshComponent> JumpHurdle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Course")
	TObjectPtr<UStaticMeshComponent> JumpHurdleTwo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Course")
	TObjectPtr<UStaticMeshComponent> JumpHurdleThree;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Course")
	TObjectPtr<UStaticMeshComponent> CrouchTunnelRoof;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Course")
	TObjectPtr<UStaticMeshComponent> CrouchTunnelLeftWall;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Course")
	TObjectPtr<UStaticMeshComponent> CrouchTunnelRightWall;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Course")
	TObjectPtr<UStaticMeshComponent> LowClimbBlock;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Course")
	TObjectPtr<UStaticMeshComponent> MediumClimbBlock;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Course")
	TObjectPtr<UStaticMeshComponent> HighClimbBlock;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Course")
	TObjectPtr<UStaticMeshComponent> ImpossibleClimbBlock;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Course")
	TObjectPtr<UStaticMeshComponent> LandingDropPlatform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Guide")
	TObjectPtr<UTextRenderComponent> GuideText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Guide")
	TObjectPtr<UTextRenderComponent> JumpGuideText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Guide")
	TObjectPtr<UTextRenderComponent> CrouchGuideText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Guide")
	TObjectPtr<UTextRenderComponent> ClimbGuideText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal|Guide")
	TObjectPtr<UTextRenderComponent> LandingGuideText;

	// Future level work belongs in normal Unreal objects or Blueprint children:
	// - visual materials and environment art;
	// - checkpoints and objectives;
	// - projectile-practice props;
	// - enemies and encounter scripting.
};
