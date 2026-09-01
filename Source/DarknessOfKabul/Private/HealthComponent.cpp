#include "HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::InitializeHealth(const float InMaxHealth)
{
	MaxHealth = FMath::Max(1.0f, InMaxHealth);
	CurrentHealth = MaxHealth;
}

float UHealthComponent::ApplyDamage(const float Amount)
{
	if (Amount <= 0.0f || IsDepleted())
	{
		return 0.0f;
	}

	// Never report more damage than the pool actually had left.
	const float AppliedDamage = FMath::Min(Amount, CurrentHealth);
	CurrentHealth -= AppliedDamage;
	return AppliedDamage;
}

float UHealthComponent::ApplyLethalDamage()
{
	return ApplyDamage(CurrentHealth);
}

void UHealthComponent::ResetHealth()
{
	CurrentHealth = MaxHealth;
}

float UHealthComponent::GetHealthPercent() const
{
	return FMath::Clamp(
		CurrentHealth / FMath::Max(MaxHealth, KINDA_SMALL_NUMBER),
		0.0f,
		1.0f
	);
}
