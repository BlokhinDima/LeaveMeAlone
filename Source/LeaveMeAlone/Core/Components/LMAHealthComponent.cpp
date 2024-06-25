// LeaveMeAlone Games. All rights reserved.


#include "Core/Components/LMAHealthComponent.h"

// Sets default values for this component's properties
ULMAHealthComponent::ULMAHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void ULMAHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	AActor* OwnerComponent = GetOwner();
	if (OwnerComponent)
	{
		OwnerComponent->OnTakeAnyDamage.AddDynamic(this, &ULMAHealthComponent::OnTakeAnyDamage);
	}

}

void ULMAHealthComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType*
	DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (IsDead()) return;
	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);
	if (IsDead())
	{
		OnDeath.Broadcast();
	}

}

bool ULMAHealthComponent::IsDead() const
{
	return Health <= 0.0f;

}