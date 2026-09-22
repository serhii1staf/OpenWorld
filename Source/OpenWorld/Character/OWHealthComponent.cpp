#include "Character/OWHealthComponent.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
UOWHealthComponent::UOWHealthComponent()
{
    SetIsReplicatedByDefault(true); PrimaryComponentTick.bCanEverTick = false;
}
void UOWHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    if (GetOwner()->HasAuthority())
    {
        Health = FMath::Max(1.f, MaxHealth);
        GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UOWHealthComponent::Damaged);
    }
}
void UOWHealthComponent::Damaged(AActor*, float Amount, const UDamageType*, AController*, AActor*)
{
    if (!GetOwner()->HasAuthority() || IsDead() || !FMath::IsFinite(Amount) || Amount <= 0.f) return;
    Health = FMath::Max(0.f, Health - Amount); Changed();
}
bool UOWHealthComponent::Restore(float Value)
{
    if (!GetOwner()->HasAuthority() || !FMath::IsFinite(Value) || Value < 0.f || Value > MaxHealth) return false;
    Health = Value; Changed(); return true;
}
void UOWHealthComponent::Changed() { OnChanged.Broadcast(Health, IsDead()); }
void UOWHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps); DOREPLIFETIME(UOWHealthComponent, Health);
}
