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
	SetUpBlock(JumpHurdle, FVector(900.0f, -350.0f, 25.0f), FVector(0.5f, 3.0f, 0.5f));

	JumpHurdleTwo = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("JumpHurdleTwo"));
	SetUpBlock(JumpHurdleTwo, FVector(1250.0f, -350.0f, 32.5f), FVector(0.5f, 3.0f, 0.65f));

	JumpHurdleThree = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("JumpHurdleThree"));
	SetUpBlock(JumpHurdleThree, FVector(1600.0f, -350.0f, 40.0f), FVector(0.5f, 3.0f, 0.8f));

	CrouchTunnelRoof = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrouchTunnelRoof"));
	SetUpBlock(CrouchTunnelRoof, FVector(2200.0f, 250.0f, 130.0f), FVector(8.0f, 3.0f, 0.25f));

	CrouchTunnelLeftWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrouchTunnelLeftWall"));
	SetUpBlock(CrouchTunnelLeftWall, FVector(2200.0f, -75.0f, 65.0f), FVector(8.0f, 0.25f, 1.3f));

	CrouchTunnelRightWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrouchTunnelRightWall"));
	SetUpBlock(CrouchTunnelRightWall, FVector(2200.0f, 575.0f, 65.0f), FVector(8.0f, 0.25f, 1.3f));

	LowClimbBlock = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LowClimbBlock"));
	SetUpBlock(LowClimbBlock, FVector(3300.0f, -450.0f, 30.0f), FVector(2.0f, 2.0f, 0.6f));

	MediumClimbBlock = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MediumClimbBlock"));
	SetUpBlock(MediumClimbBlock, FVector(3300.0f, 0.0f, 50.0f), FVector(2.0f, 2.0f, 1.0f));

	HighClimbBlock = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HighClimbBlock"));
	SetUpBlock(HighClimbBlock, FVector(3300.0f, 450.0f, 65.0f), FVector(2.0f, 2.0f, 1.3f));

	ImpossibleClimbBlock = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ImpossibleClimbBlock"));
	SetUpBlock(ImpossibleClimbBlock, FVector(4000.0f, 450.0f, 90.0f), FVector(2.0f, 2.0f, 1.8f));

	LandingDropPlatform = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LandingDropPlatform"));
	SetUpBlock(LandingDropPlatform, FVector(4700.0f, -300.0f, 60.0f), FVector(7.0f, 4.0f, 1.2f));

	GuideText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("GuideText"));
	GuideText->SetupAttachment(SceneRoot);
	GuideText->SetRelativeLocation(FVector(450.0f, 0.0f, 260.0f));
	GuideText->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	GuideText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	GuideText->SetWorldSize(30.0f);
	GuideText->SetTextRenderColor(FColor::Cyan);
	GuideText->SetText(FText::FromString(
		TEXT("MOVEMENT TRAINING  |  WALK  LOOK  SPRINT  JUMP  CROUCH  CLIMB")));

	const auto SetUpGuide = [this](UTextRenderComponent* Text, const FVector& Location, const TCHAR* Message)
	{
		Text->SetupAttachment(SceneRoot);
		Text->SetRelativeLocation(Location);
		Text->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
		Text->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
		Text->SetWorldSize(24.0f);
		Text->SetTextRenderColor(FColor::Yellow);
		Text->SetText(FText::FromString(Message));
	};

	JumpGuideText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("JumpGuideText"));
	SetUpGuide(JumpGuideText, FVector(1250.0f, -350.0f, 190.0f), TEXT("JUMP TESTS  |  50cm  65cm  80cm"));

	CrouchGuideText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("CrouchGuideText"));
	SetUpGuide(CrouchGuideText, FVector(2200.0f, 250.0f, 205.0f), TEXT("CROUCH TUNNEL  |  HOLD LEFT CTRL"));

	ClimbGuideText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ClimbGuideText"));
	SetUpGuide(ClimbGuideText, FVector(3550.0f, 0.0f, 250.0f), TEXT("CLIMB: 60/100/130cm POSSIBLE  |  180cm NOT POSSIBLE"));

	LandingGuideText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("LandingGuideText"));
	SetUpGuide(LandingGuideText, FVector(4700.0f, -300.0f, 230.0f), TEXT("LANDING TEST  |  CLIMB UP, THEN DROP"));
}
