#include "Pickups/DOKStonePickup.h"

#include "Character/DOKCharacter.h"
#include "Components/DOKSlingshotComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ADOKStonePickup::ADOKStonePickup()
{
	PrimaryActorTick.bCanEverTick = false;

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	SetRootComponent(InteractionSphere);
	InteractionSphere->InitSphereRadius(45.0f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionSphere->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(InteractionSphere);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupMesh->SetRelativeScale3D(FVector(0.18f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		PickupMesh->SetStaticMesh(SphereMesh.Object);
	}
}

void ADOKStonePickup::BeginPlay()
{
	Super::BeginPlay();
	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ADOKStonePickup::HandleBeginOverlap);
}

bool ADOKStonePickup::Collect(ADOKCharacter* Character)
{
	if (!Character || IsActorBeingDestroyed())
	{
		return false;
	}

	UDOKSlingshotComponent* Slingshot = Character->GetSlingshot();
	if (!Slingshot || Slingshot->AddAmmo(StoneAmount) <= 0)
	{
		return false;
	}

	Destroy();
	return true;
}

void ADOKStonePickup::HandleBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bAutoCollectOnOverlap)
	{
		Collect(Cast<ADOKCharacter>(OtherActor));
	}
}
