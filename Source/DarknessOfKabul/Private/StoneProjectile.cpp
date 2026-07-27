#include "StoneProjectile.h"
#include "Components/SphereComponent.h"

AStoneProjectile::AStoneProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("ProjectileCollision"));

	SetRootComponent(Collision);

	Collision->InitSphereRadius(5.0f);
	Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	Collision->SetSimulatePhysics(false);
}
