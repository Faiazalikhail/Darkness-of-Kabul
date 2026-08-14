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
#include "KinematicsLibrary.h"
#include "KabulGameMode.h"
#include "KabulPrototypeUI.h"
#include "Kismet/GameplayStatics.h"
#include "SlingshotAimGuideComponent.h"
#include "StoneProjectile.h"
#include "UObject/ConstructorHelpers.h"
#include "ZombieCharacter.h"
#include "EngineUtils.h"

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
	InitialPlayerTransform = GetActorTransform();
	InitialControlRotation = Controller
		? Controller->GetControlRotation()
		: GetActorRotation();

	UpdateLocomotionState();
}

void APlayerCharacter::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Some Blueprint game-mode spawn orders possess the pawn after its initial
	// BeginPlay/PawnClientRestart callbacks. Retry until the local controller is
	// available; the guard makes this a one-time UI initialization.
	if (!PrototypeUI)
	{
		InitializePrototypeUI();
	}

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
	InitializePrototypeUI();

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

	PlayerInputComponent->BindKey(
		EKeys::R,
		IE_Pressed,
		this,
		&APlayerCharacter::ResetPrototype
	);

	FInputKeyBinding& PauseBinding = PlayerInputComponent->BindKey(
		EKeys::Escape,
		IE_Pressed,
		this,
		&APlayerCharacter::TogglePauseMenu
	);
	PauseBinding.bExecuteWhenPaused = true;
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

	const float LaunchSpeed =
		UKinematicsLibrary::CalculateLaunchSpeed(
			HeldTime,
			MinimumChargeTime,
			MaximumChargeTime,
			MinimumLaunchSpeed,
			MaximumLaunchSpeed
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

	FVector SpawnLocation;
	FVector LaunchDirection;

	if (!ComputeSlingshotAim(SpawnLocation, LaunchDirection))
	{
		return;
	}

	const FRotator SpawnRotation = LaunchDirection.Rotation();

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
		FiredStone->Launch(LaunchDirection, LaunchSpeed);
	}
}

float APlayerCharacter::GetSlingshotPullAmount() const
{
	if (!bIsAiming || !bChargingStone)
	{
		return 0.0f;
	}

	return UKinematicsLibrary::CalculateChargeAmount(
		GetSlingshotHeldTime(),
		MinimumChargeTime,
		MaximumChargeTime
	);
}

bool APlayerCharacter::IsSlingshotShotReady() const
{
	return bIsAiming
		&& bChargingStone
		&& GetSlingshotHeldTime() >= MinimumChargeTime;
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
	if (!bIsAiming
		|| !bChargingStone
		|| GetSlingshotHeldTime() < MaximumDrawDuration)
	{
		return;
	}

	// Preserve the original overdraw consequence: the pull fails, aim resets,
	// and the camera kick tells the player to release before aiming again.
	bChargingStone = false;
	bIsAiming = false;
	LandingShakeElapsed = 0.0f;
	LandingShakeStrength = 0.65f;
}

bool APlayerCharacter::ComputeSlingshotAim(
	FVector& OutLaunchLocation,
	FVector& OutLaunchDirection
) const
{
	const UWorld* World = GetWorld();

	if (!World || !FirstPersonCameraComponent)
	{
		return false;
	}

	const FVector CameraLocation =
		FirstPersonCameraComponent->GetComponentLocation();
	const FVector CameraForward =
		FirstPersonCameraComponent->GetForwardVector();
	const FVector CameraRight =
		FirstPersonCameraComponent->GetRightVector();
	const FVector CameraUp =
		FirstPersonCameraComponent->GetUpVector();

	const FVector TraceEnd =
		CameraLocation + CameraForward * AimTargetDistance;

	FCollisionQueryParams QueryParameters(
		SCENE_QUERY_STAT(SlingshotAimTarget),
		false,
		this
	);
	QueryParameters.AddIgnoredActor(this);

	FHitResult AimHit;
	const bool bHit = World->LineTraceSingleByChannel(
		AimHit,
		CameraLocation,
		TraceEnd,
		ECC_Visibility,
		QueryParameters
	);

	const FVector TargetPoint =
		bHit ? AimHit.ImpactPoint : TraceEnd;

	OutLaunchLocation =
		CameraLocation
		+ CameraForward * LaunchOffset.X
		+ CameraRight * LaunchOffset.Y
		+ CameraUp * LaunchOffset.Z;

	OutLaunchDirection =
		(TargetPoint - OutLaunchLocation).GetSafeNormal();

	return !OutLaunchDirection.IsNearlyZero();
}

bool APlayerCharacter::GetSlingshotTrajectory(
	FSlingshotTrajectoryPrediction& OutPrediction
) const
{
	OutPrediction = FSlingshotTrajectoryPrediction();

	if (!bIsAiming || !SlingshotAimGuide || !StoneProjectileClass)
	{
		return false;
	}

	FVector LaunchLocation;
	FVector LaunchDirection;

	if (!ComputeSlingshotAim(LaunchLocation, LaunchDirection))
	{
		return false;
	}

	const float HeldTime = GetSlingshotHeldTime();
	const float ReadyAmount = FMath::Clamp(
		HeldTime / FMath::Max(MinimumChargeTime, KINDA_SMALL_NUMBER),
		0.0f,
		1.0f
	);
	const float PreviewSpeed = IsSlingshotShotReady()
		? UKinematicsLibrary::CalculateLaunchSpeed(
			HeldTime,
			MinimumChargeTime,
			MaximumChargeTime,
			MinimumLaunchSpeed,
			MaximumLaunchSpeed
		)
		: FMath::Lerp(
			MinimumLaunchSpeed * 0.35f,
			MinimumLaunchSpeed,
			ReadyAmount
		);

	const AStoneProjectile* ProjectileDefaults =
		StoneProjectileClass->GetDefaultObject<AStoneProjectile>();

	if (!ProjectileDefaults)
	{
		return false;
	}

	return SlingshotAimGuide->PredictTrajectory(
		LaunchLocation,
		LaunchDirection * PreviewSpeed,
		ProjectileDefaults->GetCollisionRadius(),
		ProjectileDefaults->GetProjectileGravityScale(),
		ProjectileDefaults->GetBounciness(),
		ProjectileDefaults->GetFriction(),
		ProjectileDefaults->GetMaximumCollisionCount(),
		ProjectileDefaults->GetMaximumLifetime(),
		this,
		OutPrediction
	);
}

void APlayerCharacter::ResetPrototype()
{
	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	bChargingStone = false;
	bIsAiming = false;
	LandingShakeElapsed = 0.0f;
	LandingShakeStrength = 0.0f;

	GetCharacterMovement()->StopMovementImmediately();
	SetActorTransform(
		InitialPlayerTransform,
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);

	if (Controller)
	{
		Controller->SetControlRotation(InitialControlRotation);
	}

	FirstPersonCameraComponent->SetRelativeLocation(CameraRestingLocation);
	FirstPersonCameraComponent->SetRelativeRotation(CameraRestingRotation);

	for (TActorIterator<AStoneProjectile> Projectile(World);
		Projectile;
		++Projectile)
	{
		Projectile->Destroy();
	}

	for (TActorIterator<AZombieCharacter> Zombie(World);
		Zombie;
		++Zombie)
	{
		Zombie->ResetReactionState();
	}

	if (AKabulGameMode* GameMode = World->GetAuthGameMode<AKabulGameMode>())
	{
		GameMode->ResetPrototypeObjectives();
	}

	ResetFeedbackEndTime = World->GetTimeSeconds() + 1.5f;
}

void APlayerCharacter::RestartPrototype()
{
	ResetPrototype();
}

float APlayerCharacter::GetResetFeedbackTimeRemaining() const
{
	const UWorld* World = GetWorld();
	return World
		? FMath::Max(0.0f, ResetFeedbackEndTime - World->GetTimeSeconds())
		: 0.0f;
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

	AddControllerYawInput(LookValue.X * LookSensitivity);
	AddControllerPitchInput(LookValue.Y * LookSensitivity);
}

void APlayerCharacter::SetLookSensitivity(const float NewSensitivity)
{
	LookSensitivity = FMath::Clamp(NewSensitivity, 0.2f, 2.0f);
}

void APlayerCharacter::SetFirstPersonFieldOfView(const float NewFieldOfView)
{
	if (FirstPersonCameraComponent)
	{
		FirstPersonCameraComponent->SetFieldOfView(
			FMath::Clamp(NewFieldOfView, 75.0f, 110.0f)
		);
	}
}

void APlayerCharacter::TogglePauseMenu()
{
	TogglePrototypePause();
}

void APlayerCharacter::InitializePrototypeUI()
{
	if (PrototypeUI || !IsLocallyControlled())
	{
		return;
	}

	if (APlayerController* PlayerController =
		Cast<APlayerController>(GetController()))
	{
		PrototypeUI = CreateWidget<UKabulPrototypeUI>(
			PlayerController,
			UKabulPrototypeUI::StaticClass()
		);

		if (PrototypeUI)
		{
			PrototypeUI->AddToViewport(100);
		}
	}
}

void APlayerCharacter::TogglePrototypePause()
{
	if (PrototypeUI)
	{
		PrototypeUI->TogglePauseMenu();
	}
}

void APlayerCharacter::ShowPrototypeCompletion()
{
	if (PrototypeUI)
	{
		PrototypeUI->ShowCompletionScreen();
	}
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
