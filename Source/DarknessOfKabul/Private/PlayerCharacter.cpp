#include "PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "SlingshotAimGuideComponent.h"
#include "StoneProjectile.h"
#include "UObject/ConstructorHelpers.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Character movement and rotation.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchWalkSpeed;
	GetCharacterMovement()->JumpZVelocity = 460.0f;
	GetCharacterMovement()->GravityScale = 1.25f;
	GetCharacterMovement()->AirControl = 0.18f;
	GetCharacterMovement()->MaxAcceleration = 1600.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1800.0f;
	GetCharacterMovement()->GroundFriction = 9.0f;
	GetCharacterMovement()->FallingLateralFriction = 0.25f;

	// First-person camera.
	FirstPersonCameraComponent =
		CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));

	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(
		FVector(-10.0f, 0.0f, 60.0f)
	);
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// First-person character mesh.
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(
		TEXT("CharacterMesh1P")
	);

	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.0f, 0.0f, -150.0f));

	// The guide is calculation and drawing only. It does not attach to the
	// camera, so it cannot alter the working camera hierarchy.
	SlingshotAimGuide =
		CreateDefaultSubobject<USlingshotAimGuideComponent>(
			TEXT("SlingshotAimGuide")
		);

	// Find the project's input assets.
	static ConstructorHelpers::FObjectFinder<UInputMappingContext>
		DefaultContext(TEXT("/Game/Input/IMC_Default.IMC_Default"));

	static ConstructorHelpers::FObjectFinder<UInputMappingContext>
		MouseContext(TEXT("/Game/Input/IMC_MouseLook.IMC_MouseLook"));

	static ConstructorHelpers::FObjectFinder<UInputAction>
		MoveInput(TEXT("/Game/Input/Actions/IA_Move.IA_Move"));

	static ConstructorHelpers::FObjectFinder<UInputAction>
		LookInput(TEXT("/Game/Input/Actions/IA_Look.IA_Look"));

	static ConstructorHelpers::FObjectFinder<UInputAction>
		MouseLookInput(TEXT("/Game/Input/Actions/IA_MouseLook.IA_MouseLook"));

	static ConstructorHelpers::FObjectFinder<UInputAction>
		JumpInput(TEXT("/Game/Input/Actions/IA_Jump.IA_Jump"));

	DefaultMappingContext = DefaultContext.Object;
	MouseLookMappingContext = MouseContext.Object;
	MoveAction = MoveInput.Object;
	LookAction = LookInput.Object;
	MouseLookAction = MouseLookInput.Object;
	JumpAction = JumpInput.Object;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	CameraRestingLocation =
		FirstPersonCameraComponent->GetRelativeLocation();

	CameraRestingRotation =
		FirstPersonCameraComponent->GetRelativeRotation();

	UpdateLocomotionState();
}

void APlayerCharacter::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateSlingshotAim();

	if (LandingShakeStrength <= 0.0f)
	{
		return;
	}

	LandingShakeElapsed += DeltaSeconds;

	const float NormalizedTime =
		LandingShakeElapsed / LandingShakeDuration;

	if (NormalizedTime >= 1.0f)
	{
		LandingShakeStrength = 0.0f;

		FirstPersonCameraComponent->SetRelativeLocation(
			CameraRestingLocation
		);

		FirstPersonCameraComponent->SetRelativeRotation(
			CameraRestingRotation
		);

		return;
	}

	const float Envelope = FMath::Square(1.0f - NormalizedTime);
	const float Wave =
		FMath::Sin(NormalizedTime * 3.0f * UE_PI);

	const float CameraDrop =
		-MaximumLandingCameraDrop
		* LandingShakeStrength
		* Envelope
		* FMath::Abs(Wave);

	const float CameraRoll =
		0.7f
		* LandingShakeStrength
		* Envelope
		* Wave;

	FirstPersonCameraComponent->SetRelativeLocation(
		CameraRestingLocation
		+ FVector(0.0f, 0.0f, CameraDrop)
	);

	FirstPersonCameraComponent->SetRelativeRotation(
		CameraRestingRotation
		+ FRotator(0.0f, 0.0f, CameraRoll)
	);
}

void APlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	APlayerController* PlayerController =
		Cast<APlayerController>(GetController());

	if (!PlayerController)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
			PlayerController->GetLocalPlayer()
		);

	if (!Subsystem)
	{
		return;
	}

	Subsystem->ClearAllMappings();

	if (DefaultMappingContext)
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	if (MouseLookMappingContext)
	{
		Subsystem->AddMappingContext(MouseLookMappingContext, 1);
	}
}

void APlayerCharacter::Landed(const FHitResult& Hit)
{
	const float ImpactSpeed =
		FMath::Max(0.0f, -GetVelocity().Z);

	Super::Landed(Hit);

	if (ImpactSpeed < LandingShakeMinimumSpeed)
	{
		return;
	}

	LandingShakeElapsed = 0.0f;

	LandingShakeStrength =
		FMath::GetMappedRangeValueClamped(
			FVector2D(LandingShakeMinimumSpeed, 1100.0f),
			FVector2D(0.25f, 1.0f),
			ImpactSpeed
		);
}

void APlayerCharacter::SetupPlayerInputComponent(
	UInputComponent* PlayerInputComponent
)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput =
		Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(
				MoveAction,
				ETriggerEvent::Triggered,
				this,
				&APlayerCharacter::Move
			);
		}

		if (LookAction)
		{
			EnhancedInput->BindAction(
				LookAction,
				ETriggerEvent::Triggered,
				this,
				&APlayerCharacter::Look
			);
		}

		if (MouseLookAction)
		{
			EnhancedInput->BindAction(
				MouseLookAction,
				ETriggerEvent::Triggered,
				this,
				&APlayerCharacter::Look
			);
		}

		if (JumpAction)
		{
			EnhancedInput->BindAction(
				JumpAction,
				ETriggerEvent::Started,
				this,
				&APlayerCharacter::StartJump
			);

			EnhancedInput->BindAction(
				JumpAction,
				ETriggerEvent::Completed,
				this,
				&APlayerCharacter::EndJump
			);
		}
	}

	PlayerInputComponent->BindKey(
		EKeys::LeftShift,
		IE_Pressed,
		this,
		&APlayerCharacter::StartSprint
	);

	PlayerInputComponent->BindKey(
		EKeys::LeftShift,
		IE_Released,
		this,
		&APlayerCharacter::StopSprint
	);

	PlayerInputComponent->BindKey(
		EKeys::LeftControl,
		IE_Pressed,
		this,
		&APlayerCharacter::StartCrouch
	);

	PlayerInputComponent->BindKey(
		EKeys::LeftControl,
		IE_Released,
		this,
		&APlayerCharacter::StopCrouch
	);

	PlayerInputComponent->BindKey(
		EKeys::LeftMouseButton,
		IE_Pressed,
		this,
		&APlayerCharacter::StartChargingStone
	);

	PlayerInputComponent->BindKey(
		EKeys::LeftMouseButton,
		IE_Released,
		this,
		&APlayerCharacter::ReleaseChargedStone
	);

	PlayerInputComponent->BindKey(
		EKeys::RightMouseButton,
		IE_Pressed,
		this,
		&APlayerCharacter::StartAiming
	);

	PlayerInputComponent->BindKey(
		EKeys::RightMouseButton,
		IE_Released,
		this,
		&APlayerCharacter::StopAiming
	);


}


void APlayerCharacter::StartAiming()
{
	bIsAiming = true;
}

void APlayerCharacter::StopAiming()
{
	bIsAiming = false;

	// Releasing aim also cancels an unfinished pull.
	bChargingStone = false;
}


void APlayerCharacter::StartChargingStone()
{
	UWorld* World = GetWorld();

	if (!World || !bIsAiming || bChargingStone)
	{
		return;
	}

	bChargingStone = true;
	ChargeStartTime = World->GetTimeSeconds();
}

void APlayerCharacter::ReleaseChargedStone()
{
	UWorld* World = GetWorld();

	if (!World || !bChargingStone)
	{
		return;
	}

	bChargingStone = false;

	const float HeldTime =
		FMath::Max(
			0.0f,
			World->GetTimeSeconds() - ChargeStartTime
		);

	// Releasing too early cancels the shot.
	if (HeldTime < MinimumChargeTime)
	{
		return;
	}

	const float ChargeRange =
		FMath::Max(
			MaximumChargeTime - MinimumChargeTime,
			KINDA_SMALL_NUMBER
		);

	const float ChargeAmount =
		FMath::Clamp(
			(HeldTime - MinimumChargeTime) / ChargeRange,
			0.0f,
			0.3f
		);

	const float LaunchSpeed =
		FMath::Lerp(
			MinimumLaunchSpeed,
			MaximumLaunchSpeed,
			ChargeAmount
		);

	FireStone(LaunchSpeed);
}


void APlayerCharacter::FireStone(const float LaunchSpeed)
{
	UWorld* World = GetWorld();

	if (!World
		|| !FirstPersonCameraComponent
		|| !StoneProjectileClass)
	{
		return;
	}

	const FVector CameraLocation =
		FirstPersonCameraComponent->GetComponentLocation();

	const FVector CameraForward =
		FirstPersonCameraComponent->GetForwardVector();

	const FVector SpawnLocation =
		CameraLocation + CameraForward * 100.0f;

	const FRotator SpawnRotation =
		FirstPersonCameraComponent->GetComponentRotation();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;
	SpawnParameters.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::
		AdjustIfPossibleButAlwaysSpawn;

	AStoneProjectile* FiredStone =
		World->SpawnActor<AStoneProjectile>(
			StoneProjectileClass,
			SpawnLocation,
			SpawnRotation,
			SpawnParameters
		);

	if (FiredStone)
	{
		FiredStone->Launch(CameraForward, LaunchSpeed);
	}
}

float APlayerCharacter::GetSlingshotPullAmount() const
{
	if (!bIsAiming || !bChargingStone)
	{
		return 0.0f;
	}

	const float ChargeRange =
		FMath::Max(
			MaximumChargeTime - MinimumChargeTime,
			KINDA_SMALL_NUMBER
		);

	return FMath::Clamp(
		(GetSlingshotHeldTime() - MinimumChargeTime) / ChargeRange,
		0.0f,
		1.0f
	);
}

float APlayerCharacter::GetSlingshotHeldTime() const
{
	const UWorld* World = GetWorld();

	if (!World || !bChargingStone)
	{
		return 0.0f;
	}

	return FMath::Max(
		0.0f,
		World->GetTimeSeconds() - ChargeStartTime
	);
}

void APlayerCharacter::UpdateSlingshotAim()
{
	const float HeldTime = GetSlingshotHeldTime();

	// Pulling past the safe limit cancels the shot and forces the player
	// to release and press the aim button again.
	if (bIsAiming
		&& bChargingStone
		&& HeldTime >= MaximumChargeTime + OverdrawGraceTime)
	{
		bChargingStone = false;
		bIsAiming = false;

		LandingShakeElapsed = 0.0f;
		LandingShakeStrength = 0.65f;
	}

	if (SlingshotAimGuide)
	{
		SlingshotAimGuide->DrawGuide(
			FirstPersonCameraComponent,
			bIsAiming,
			bChargingStone,
			HeldTime,
			MinimumChargeTime,
			MaximumChargeTime,
			MinimumLaunchSpeed,
			MaximumLaunchSpeed
		);
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	if (!Controller)
	{
		return;
	}

	const FVector2D MovementValue =
		Value.Get<FVector2D>();

	const FRotator ControlRotation =
		GetControlRotation();

	const FRotator YawRotation(
		0.0f,
		ControlRotation.Yaw,
		0.0f
	);

	const FVector ForwardDirection =
		FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	const FVector RightDirection =
		FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(
		ForwardDirection,
		MovementValue.Y
	);

	AddMovementInput(
		RightDirection,
		MovementValue.X
	);
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	if (!Controller)
	{
		return;
	}

	const FVector2D LookValue =
		Value.Get<FVector2D>();

	AddControllerYawInput(LookValue.X);
	AddControllerPitchInput(LookValue.Y);
}

void APlayerCharacter::StartJump()
{
	if (!TryClimb())
	{
		Jump();
	}
}

void APlayerCharacter::EndJump()
{
	StopJumping();
}

void APlayerCharacter::StartSprint()
{
	bSprintHeld = true;
	UpdateLocomotionState();
}

void APlayerCharacter::StopSprint()
{
	bSprintHeld = false;
	UpdateLocomotionState();
}

void APlayerCharacter::StartCrouch()
{
	bSprintHeld = false;

	if (GetCharacterMovement()->IsMovingOnGround())
	{
		Crouch();
	}
}

void APlayerCharacter::StopCrouch()
{
	UnCrouch();
}

void APlayerCharacter::OnStartCrouch(
	const float HalfHeightAdjust,
	const float ScaledHalfHeightAdjust
)
{
	Super::OnStartCrouch(
		HalfHeightAdjust,
		ScaledHalfHeightAdjust
	);

	UpdateLocomotionState();
}

void APlayerCharacter::OnEndCrouch(
	const float HalfHeightAdjust,
	const float ScaledHalfHeightAdjust
)
{
	Super::OnEndCrouch(
		HalfHeightAdjust,
		ScaledHalfHeightAdjust
	);

	UpdateLocomotionState();
}

void APlayerCharacter::UpdateLocomotionState()
{
	UCharacterMovementComponent* Movement =
		GetCharacterMovement();

	Movement->MaxWalkSpeedCrouched = CrouchWalkSpeed;

	if (bIsCrouched)
	{
		Movement->MaxWalkSpeed = WalkSpeed;
		MovementNoiseMultiplier = CrouchNoiseMultiplier;
	}
	else if (bSprintHeld)
	{
		Movement->MaxWalkSpeed = SprintSpeed;
		MovementNoiseMultiplier = SprintNoiseMultiplier;
	}
	else
	{
		Movement->MaxWalkSpeed = WalkSpeed;
		MovementNoiseMultiplier = WalkNoiseMultiplier;
	}
}

bool APlayerCharacter::TryClimb()
{
	UWorld* World = GetWorld();
	UCapsuleComponent* Capsule = GetCapsuleComponent();

	if (!World
		|| !Capsule
		|| bIsCrouched
		|| !GetCharacterMovement()->IsMovingOnGround())
	{
		return false;
	}

	const FRotator YawRotation(
		0.0f,
		GetControlRotation().Yaw,
		0.0f
	);

	const FVector Forward =
		FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	const FVector ActorLocation =
		GetActorLocation();

	const float CapsuleHalfHeight =
		Capsule->GetScaledCapsuleHalfHeight();

	const float CapsuleRadius =
		Capsule->GetScaledCapsuleRadius();

	const float FeetHeight =
		ActorLocation.Z - CapsuleHalfHeight;

	FCollisionQueryParams QueryParameters(
		SCENE_QUERY_STAT(PlayerClimb),
		false,
		this
	);

	QueryParameters.AddIgnoredActor(this);

	const FVector WallTraceStart =
		ActorLocation + FVector::UpVector * 35.0f;

	const FVector WallTraceEnd =
		WallTraceStart + Forward * ClimbReach;

	FHitResult WallHit;

	if (!World->LineTraceSingleByChannel(
		WallHit,
		WallTraceStart,
		WallTraceEnd,
		ECC_Visibility,
		QueryParameters))
	{
		return false;
	}

	const FVector IntoLedge =
		(-WallHit.ImpactNormal).GetSafeNormal2D();

	const FVector TopTraceStart =
		WallHit.ImpactPoint
		+ IntoLedge * ClimbLandingInset
		+ FVector::UpVector
		* (MaximumClimbHeight + CapsuleHalfHeight);

	const FVector TopTraceEnd(
		TopTraceStart.X,
		TopTraceStart.Y,
		FeetHeight + MinimumClimbHeight
	);

	FHitResult TopHit;

	if (!World->LineTraceSingleByChannel(
		TopHit,
		TopTraceStart,
		TopTraceEnd,
		ECC_Visibility,
		QueryParameters))
	{
		return false;
	}

	const float LedgeHeight =
		TopHit.ImpactPoint.Z - FeetHeight;

	if (LedgeHeight < MinimumClimbHeight
		|| LedgeHeight > MaximumClimbHeight
		|| TopHit.ImpactNormal.Z
		< GetCharacterMovement()->GetWalkableFloorZ())
	{
		return false;
	}

	const FVector TargetLocation =
		TopHit.ImpactPoint
		+ IntoLedge * CapsuleRadius
		+ FVector::UpVector
		* (CapsuleHalfHeight + 2.0f);

	const FCollisionShape CapsuleShape =
		FCollisionShape::MakeCapsule(
			CapsuleRadius,
			CapsuleHalfHeight
		);

	if (World->OverlapBlockingTestByChannel(
		TargetLocation,
		FQuat::Identity,
		ECC_Pawn,
		CapsuleShape,
		QueryParameters))
	{
		return false;
	}

	GetCharacterMovement()->StopMovementImmediately();

	FHitResult MoveHit;

	SetActorLocation(
		TargetLocation,
		true,
		&MoveHit,
		ETeleportType::None
	);

	return !MoveHit.bBlockingHit;
}
