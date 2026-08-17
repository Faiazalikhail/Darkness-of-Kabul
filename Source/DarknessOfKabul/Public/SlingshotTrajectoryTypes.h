#pragma once

#include "CoreMinimal.h"

/** One readable impact in the slingshot preview. */
struct FSlingshotTrajectoryImpact
{
	FVector ImpactPoint = FVector::ZeroVector;
	FVector IncomingGuidePoint = FVector::ZeroVector;
	FVector OutgoingGuidePoint = FVector::ZeroVector;
	TWeakObjectPtr<AActor> HitActor;
	bool bHitZombie = false;
	bool bHasOutgoingSegment = false;
};

/** Compact trajectory result used by the UMG marker renderer. */
struct FSlingshotTrajectoryPrediction
{
	TArray<FSlingshotTrajectoryImpact> Impacts;
	FVector FinalPoint = FVector::ZeroVector;
	bool bEndedOnCollision = false;
};
