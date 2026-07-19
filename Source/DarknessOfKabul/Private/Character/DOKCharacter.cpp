#include "Character/DOKCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/DOKSlingshotComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "Engine/World.h"
#include "Game/DOKGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "Pickups/DOKStonePickup.h"
#include "UObject/ConstructorHelpers.h"

ADOKCharacter::ADOKCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(34.0f, 96.0f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(-10.0f, 0.0f, 64.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->SetFieldOfView(NormalFieldOfView);

	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(FirstPersonCamera);
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonMesh->CastShadow = false;
	GetMesh()->SetOwnerNoSee(true);

	SlingshotComponent = CreateDefaultSubobject<UDOKSlingshotComponent>(TEXT("SlingshotComponent"));

	static ConstructorHelpers::FObjectFinder<UInputAction> MoveInput(
		TEXT("/Game/Input/Actions/IA_Move.IA_Move"));
	MoveAction = MoveInput.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> LookInput(
		TEXT("/Game/Input/Actions/IA_Look.IA_Look"));
	LookAction = LookInput.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> MouseLookInput(
		TEXT("/Game/Input/Actions/IA_MouseLook.IA_MouseLook"));
	MouseLookAction = MouseLookInput.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> JumpInput(
		TEXT("/Game/Input/Actions/IA_Jump.IA_Jump"));
	JumpAction = JumpInput.Object;
}

void ADOKCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaximumHealth;
	UpdateMovementSpeed();
	OnHealthChanged.Broadcast(GetHealthPercent());
}

void ADOKCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADOKCharacter::Move);
		}
		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADOKCharacter::Look);
		}
		if (MouseLookAction)
		{
			EnhancedInput->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ADOKCharacter::Look);
		}
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ADOKCharacter::StartJump);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ADOKCharacter::StopJump);
		}
	}

	PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &ADOKCharacter::StartAim);
	PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Released, this, &ADOKCharacter::StopAim);
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ADOKCharacter::StartFire);
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &ADOKCharacter::ReleaseFire);
	PlayerInputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &ADOKCharacter::StartSprint);
	PlayerInputComponent->BindKey(EKeys::LeftShift, IE_Released, this, &ADOKCharacter::StopSprint);
	PlayerInputComponent->BindKey(EKeys::E, IE_Pressed, this, &ADOKCharacter::Interact);
	PlayerInputComponent->BindKey(EKeys::T, IE_Pressed, this, &ADOKCharacter::ToggleTrajectoryGuide);
	PlayerInputComponent->BindKey(EKeys::R, IE_Pressed, this, &ADOKCharacter::RestartLevel);
}

float ADOKCharacter::TakeDamage(
	const float DamageAmount,
	const FDamageEvent& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (AppliedDamage <= 0.0f || CurrentHealth <= 0.0f)
	{
		return 0.0f;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - AppliedDamage, 0.0f, MaximumHealth);
	OnHealthChanged.Broadcast(GetHealthPercent());

	if (CurrentHealth <= 0.0f)
	{
		DisableInput(Cast<APlayerController>(GetController()));
		if (ADOKGameMode* GameMode = GetWorld()->GetAuthGameMode<ADOKGameMode>())
		{
			GameMode->LoseGame(TEXT("The shepherd was overwhelmed."));
		}
	}

	return AppliedDamage;
}

float ADOKCharacter::GetHealthPercent() const
{
	return MaximumHealth > 0.0f ? CurrentHealth / MaximumHealth : 0.0f;
}

void ADOKCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Movement = Value.Get<FVector2D>();
	AddMovementInput(GetActorForwardVector(), Movement.Y);
	AddMovementInput(GetActorRightVector(), Movement.X);
}

void ADOKCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(LookAxis.Y);
}

void ADOKCharacter::StartJump()
{
	Jump();
}

void ADOKCharacter::StopJump()
{
	StopJumping();
}

void ADOKCharacter::StartAim()
{
	bIsAiming = true;
	FirstPersonCamera->SetFieldOfView(AimFieldOfView);
	UpdateMovementSpeed();
}

void ADOKCharacter::StopAim()
{
	bIsAiming = false;
	SlingshotComponent->CancelDrawing();
	FirstPersonCamera->SetFieldOfView(NormalFieldOfView);
	UpdateMovementSpeed();
}

void ADOKCharacter::StartFire()
{
	if (bIsAiming)
	{
		SlingshotComponent->StartDrawing();
	}
}

void ADOKCharacter::ReleaseFire()
{
	if (SlingshotComponent->IsDrawing())
	{
		SlingshotComponent->ReleaseShot(FirstPersonCamera->GetForwardVector());
	}
}

void ADOKCharacter::StartSprint()
{
	bIsSprinting = true;
	UpdateMovementSpeed();
}

void ADOKCharacter::StopSprint()
{
	bIsSprinting = false;
	UpdateMovementSpeed();
}

void ADOKCharacter::Interact()
{
	if (!GetWorld())
	{
		return;
	}

	const FVector Start = FirstPersonCamera->GetComponentLocation();
	const FVector End = Start + FirstPersonCamera->GetForwardVector() * InteractionDistance;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(DOKInteraction), false, this);
	Params.AddIgnoredActor(this);

	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel2, Params))
	{
		if (ADOKStonePickup* Pickup = Cast<ADOKStonePickup>(Hit.GetActor()))
		{
			Pickup->Collect(this);
		}
	}
}

void ADOKCharacter::ToggleTrajectoryGuide()
{
	SlingshotComponent->ToggleTrajectoryGuide();
}

void ADOKCharacter::RestartLevel()
{
	if (ADOKGameMode* GameMode = GetWorld()->GetAuthGameMode<ADOKGameMode>())
	{
		GameMode->RestartCurrentLevel();
	}
}

void ADOKCharacter::UpdateMovementSpeed()
{
	float DesiredSpeed = WalkSpeed;
	if (bIsAiming)
	{
		DesiredSpeed = AimSpeed;
	}
	else if (bIsSprinting)
	{
		DesiredSpeed = SprintSpeed;
	}
	GetCharacterMovement()->MaxWalkSpeed = DesiredSpeed;
}
