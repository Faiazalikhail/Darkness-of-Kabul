#include "TraversalCourse.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ATraversalCourse::ATraversalCourse()
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Cube = CubeAsset.Succeeded() ? CubeAsset.Object : nullptr;

	const auto SetUpBlock = [this, Cube](
		UStaticMeshComponent* Block,
		const FVector& Location,
		const FVector& Scale)
	{
		Block->SetupAttachment(SceneRoot);
		Block->SetStaticMesh(Cube);
		Block->SetRelativeLocation(Location);
		Block->SetRelativeScale3D(Scale);
		Block->SetCollisionProfileName(TEXT("BlockAll"));
	};

	JumpHurdle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("JumpHurdle"));
	SetUpBlock(JumpHurdle, FVector(900.0f, -350.0f, 40.0f), FVector(0.5f, 3.0f, 0.8f));

	CrouchTunnelRoof = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrouchTunnelRoof"));
	SetUpBlock(CrouchTunnelRoof, FVector(1600.0f, 250.0f, 130.0f), FVector(5.0f, 3.0f, 0.25f));

	CrouchTunnelLeftWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrouchTunnelLeftWall"));
	SetUpBlock(CrouchTunnelLeftWall, FVector(1600.0f, -75.0f, 65.0f), FVector(5.0f, 0.25f, 1.3f));

	CrouchTunnelRightWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrouchTunnelRightWall"));
	SetUpBlock(CrouchTunnelRightWall, FVector(1600.0f, 575.0f, 65.0f), FVector(5.0f, 0.25f, 1.3f));

	LowClimbBlock = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LowClimbBlock"));
	SetUpBlock(LowClimbBlock, FVector(2400.0f, -450.0f, 30.0f), FVector(2.0f, 2.0f, 0.6f));

	MediumClimbBlock = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MediumClimbBlock"));
	SetUpBlock(MediumClimbBlock, FVector(2400.0f, 0.0f, 50.0f), FVector(2.0f, 2.0f, 1.0f));

	HighClimbBlock = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HighClimbBlock"));
	SetUpBlock(HighClimbBlock, FVector(2400.0f, 450.0f, 65.0f), FVector(2.0f, 2.0f, 1.3f));

	GuideText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("GuideText"));
	GuideText->SetupAttachment(SceneRoot);
	GuideText->SetRelativeLocation(FVector(450.0f, 0.0f, 260.0f));
	GuideText->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	GuideText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	GuideText->SetWorldSize(30.0f);
	GuideText->SetTextRenderColor(FColor::Cyan);
	GuideText->SetText(FText::FromString(
		TEXT("MILESTONE 1  |  WALK  LOOK  SPRINT  JUMP  CROUCH  CLIMB")));
}
