#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "UObject/ConstructorHelpers.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Configure rotation settings
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
	GetCharacterMovement()->MaxAcceleration = 1300.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1800.0f;
	GetCharacterMovement()->GroundFriction = 9.0f;
	GetCharacterMovement()->FallingLateralFriction = 0.25f;

	// Create and setup the First Person Camera
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create and setup the First Person Mesh (arms)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	// Safe native defaults keep the C++ pawn playable even before a Blueprint
	// child assigns its own input assets.
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultContext(
		TEXT("/Game/Input/IMC_Default.IMC_Default"));
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MouseContext(
		TEXT("/Game/Input/IMC_MouseLook.IMC_MouseLook"));
	static ConstructorHelpers::FObjectFinder<UInputAction> MoveInput(
		TEXT("/Game/Input/Actions/IA_Move.IA_Move"));
	static ConstructorHelpers::FObjectFinder<UInputAction> LookInput(
		TEXT("/Game/Input/Actions/IA_Look.IA_Look"));
	static ConstructorHelpers::FObjectFinder<UInputAction> MouseLookInput(
		TEXT("/Game/Input/Actions/IA_MouseLook.IA_MouseLook"));
	static ConstructorHelpers::FObjectFinder<UInputAction> JumpInput(
		TEXT("/Game/Input/Actions/IA_Jump.IA_Jump"));

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
	CameraRestingLocation = FirstPersonCameraComponent->GetRelativeLocation();
	CameraRestingRotation = FirstPersonCameraComponent->GetRelativeRotation();
	UpdateLocomotionState();
}

void APlayerCharacter::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (LandingShakeStrength <= 0.0f)
	{
		return;
	}

	LandingShakeElapsed += DeltaSeconds;
	const float NormalizedTime = LandingShakeElapsed / LandingShakeDuration;
	if (NormalizedTime >= 1.0f)
	{
		LandingShakeStrength = 0.0f;
		FirstPersonCameraComponent->SetRelativeLocation(CameraRestingLocation);
		FirstPersonCameraComponent->SetRelativeRotation(CameraRestingRotation);
		return;
	}

	// A fast damped dip gives impact weight without disturbing player aim.
	const float Envelope = FMath::Square(1.0f - NormalizedTime);
	const float Wave = FMath::Sin(NormalizedTime * 3.0f * UE_PI);
	const float CameraDrop = -MaximumLandingCameraDrop * LandingShakeStrength * Envelope * FMath::Abs(Wave);
	const float CameraRoll = 0.7f * LandingShakeStrength * Envelope * Wave;
	FirstPersonCameraComponent->SetRelativeLocation(CameraRestingLocation + FVector(0.0f, 0.0f, CameraDrop));
	FirstPersonCameraComponent->SetRelativeRotation(CameraRestingRotation + FRotator(0.0f, 0.0f, CameraRoll));
}

void APlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	// Register Input Mapping Contexts here to ensure the controller is fully ready
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
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
	}
}

void APlayerCharacter::Landed(const FHitResult& Hit)
{
	const float ImpactSpeed = FMath::Max(0.0f, -GetVelocity().Z);
	Super::Landed(Hit);

	if (ImpactSpeed >= LandingShakeMinimumSpeed)
	{
		LandingShakeElapsed = 0.0f;
		LandingShakeStrength = FMath::GetMappedRangeValueClamped(
			FVector2D(LandingShakeMinimumSpeed, 1100.0f),
			FVector2D(0.25f, 1.0f),
			ImpactSpeed);
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		}

		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		}

		if (MouseLookAction)
		{
			EnhancedInput->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		}

		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerCharacter::StartJump);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlayerCharacter::EndJump);
		}
	}

	// These actions stay direct and readable. They can become Input
	// Action assets later
	PlayerInputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &APlayerCharacter::StartSprint);
	PlayerInputComponent->BindKey(EKeys::LeftShift, IE_Released, this, &APlayerCharacter::StopSprint);
	PlayerInputComponent->BindKey(EKeys::LeftControl, IE_Pressed, this, &APlayerCharacter::StartCrouch);
	PlayerInputComponent->BindKey(EKeys::LeftControl, IE_Released, this, &APlayerCharacter::StopCrouch);
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		const FVector2D MovementValue = Value.Get<FVector2D>();

		const FRotator ControlRotation = GetControlRotation();
		const FRotator YawRotation(0.0, ControlRotation.Yaw, 0.0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementValue.Y);
		AddMovementInput(RightDirection, MovementValue.X);
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		const FVector2D LookValue = Value.Get<FVector2D>();

		AddControllerYawInput(LookValue.X);
		AddControllerPitchInput(LookValue.Y);
	}
}

void APlayerCharacter::StartJump()
{
	// Space first tries a short ledge climb. If no valid ledge exists, normal
	// CharacterMovement jumping handles the vertical launch and gravity.
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

void APlayerCharacter::OnStartCrouch(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	UpdateLocomotionState();
}

void APlayerCharacter::OnEndCrouch(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	UpdateLocomotionState();
}

void APlayerCharacter::UpdateLocomotionState()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
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
	if (!World || !Capsule || bIsCrouched || !GetCharacterMovement()->IsMovingOnGround())
	{
		return false;
	}

	const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector ActorLocation = GetActorLocation();
	const float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	const float CapsuleRadius = Capsule->GetScaledCapsuleRadius();
	const float FeetHeight = ActorLocation.Z - CapsuleHalfHeight;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PlayerClimb), false, this);
	QueryParams.AddIgnoredActor(this);

	// First trace: find a wall close to the player's chest.
	const FVector WallTraceStart = ActorLocation + FVector::UpVector * 35.0f;
	const FVector WallTraceEnd = WallTraceStart + Forward * ClimbReach;
	FHitResult WallHit;
	if (!World->LineTraceSingleByChannel(WallHit, WallTraceStart, WallTraceEnd, ECC_Visibility, QueryParams))
	{
		return false;
	}

	// Second trace: start above the wall and search downward for a walkable top.
	const FVector IntoLedge = (-WallHit.ImpactNormal).GetSafeNormal2D();
	const FVector TopTraceStart = WallHit.ImpactPoint
		+ IntoLedge * ClimbLandingInset
		+ FVector::UpVector * (MaximumClimbHeight + CapsuleHalfHeight);
	const FVector TopTraceEnd = FVector(TopTraceStart.X, TopTraceStart.Y, FeetHeight + MinimumClimbHeight);
	FHitResult TopHit;
	if (!World->LineTraceSingleByChannel(TopHit, TopTraceStart, TopTraceEnd, ECC_Visibility, QueryParams))
	{
		return false;
	}

	const float LedgeHeight = TopHit.ImpactPoint.Z - FeetHeight;
	if (LedgeHeight < MinimumClimbHeight
		|| LedgeHeight > MaximumClimbHeight
		|| TopHit.ImpactNormal.Z < GetCharacterMovement()->GetWalkableFloorZ())
	{
		return false;
	}

	const FVector TargetLocation = TopHit.ImpactPoint
		+ IntoLedge * CapsuleRadius
		+ FVector::UpVector * (CapsuleHalfHeight + 2.0f);
	const FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);
	if (World->OverlapBlockingTestByChannel(
		TargetLocation,
		FQuat::Identity,
		ECC_Pawn,
		CapsuleShape,
		QueryParams))
	{
		return false;
	}

	// A future animation can interpolate to this target. This prototype uses a
	// swept move so the capsule never teleports through blocking geometry.
	GetCharacterMovement()->StopMovementImmediately();
	FHitResult MoveHit;
	SetActorLocation(TargetLocation, true, &MoveHit, ETeleportType::None);
	return !MoveHit.bBlockingHit;
}
