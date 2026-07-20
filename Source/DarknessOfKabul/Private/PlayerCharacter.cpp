#include "PlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "UObject/ConstructorHelpers.h"

APlayerCharacter::APlayerCharacter()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultContextObject(
		TEXT("/Game/Input/IMC_Default.IMC_Default"));
	DefaultMappingContext = DefaultContextObject.Object;

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MouseContextObject(
		TEXT("/Game/Input/IMC_MouseLook.IMC_MouseLook"));
	MouseLookMappingContext = MouseContextObject.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> MoveActionObject(
		TEXT("/Game/Input/Actions/IA_Move.IA_Move"));
	MoveAction = MoveActionObject.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> LookActionObject(
		TEXT("/Game/Input/Actions/IA_Look.IA_Look"));
	LookAction = LookActionObject.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> MouseLookActionObject(
		TEXT("/Game/Input/Actions/IA_MouseLook.IA_MouseLook"));
	MouseLookAction = MouseLookActionObject.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> JumpActionObject(
		TEXT("/Game/Input/Actions/IA_Jump.IA_Jump"));
	JumpAction = JumpActionObject.Object;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (!InputSubsystem)
	{
		return;
	}

	if (DefaultMappingContext)
	{
		InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	if (MouseLookMappingContext)
	{
		InputSubsystem->AddMappingContext(MouseLookMappingContext, 1);
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput)
	{
		return;
	}

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

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementValue = Value.Get<FVector2D>();
	const FRotator ControlRotation = GetControlRotation();
	const FRotator YawRotation(0.0, ControlRotation.Yaw, 0.0);

	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), MovementValue.Y);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), MovementValue.X);
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookValue = Value.Get<FVector2D>();
	AddControllerYawInput(LookValue.X);
	AddControllerPitchInput(LookValue.Y);
}

void APlayerCharacter::StartJump()
{
	Jump();
}

void APlayerCharacter::EndJump()
{
	StopJumping();
}
