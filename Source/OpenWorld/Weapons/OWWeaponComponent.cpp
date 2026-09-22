#include "Weapons/OWWeaponComponent.h"
#include "Weapons/OWWeaponDefinition.h"
#include "Weapons/OWBallisticsSubsystem.h"
#include "Character/OWHealthComponent.h"
#include "Inventory/OWInventoryComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AISense_Hearing.h"
UOWWeaponComponent::UOWWeaponComponent() { SetIsReplicatedByDefault(true); PrimaryComponentTick.bCanEverTick = false; }
void UOWWeaponComponent::BeginPlay()
{
    Super::BeginPlay();
    if (GetOwner()->HasAuthority() && Definition) Magazine = FMath::Clamp(Definition->MagazineSize, 1, 200);
}
UOWInventoryComponent* UOWWeaponComponent::Inventory() const
{
    const APawn* P = Cast<APawn>(GetOwner());
    const APlayerState* S = P ? P->GetPlayerState() : nullptr;
    return S ? S->FindComponentByClass<UOWInventoryComponent>() : nullptr;
}
void UOWWeaponComponent::StartFire()
{
    if (!GetOwner()->HasAuthority() || !Definition || GetWorld()->GetTimerManager().IsTimerActive(FireTimer)) return;
    FireOne();
    if (Definition->bAutomatic)
        GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &UOWWeaponComponent::FireOne, FMath::Max(0.05f, Definition->ShotInterval), true);
}
void UOWWeaponComponent::StopFire() { GetWorld()->GetTimerManager().ClearTimer(FireTimer); }
void UOWWeaponComponent::FireOne()
{
    APawn* Pawn = Cast<APawn>(GetOwner());
    const UOWHealthComponent* Health = GetOwner()->FindComponentByClass<UOWHealthComponent>();
    const double Now = GetWorld()->GetTimeSeconds();
    if (!Pawn || !Pawn->HasAuthority() || !Pawn->GetController() || !Definition || (Health && Health->IsDead()) || bReloading || Magazine <= 0 || Now < NextShotAt) return;
    FVector Origin; FRotator Rotation; Pawn->GetActorEyesViewPoint(Origin, Rotation);
    const FVector Direction = FMath::VRandCone(Rotation.Vector(), FMath::DegreesToRadians(FMath::Clamp(Definition->SpreadDegrees, 0.f, 20.f)));
    if (GetWorld()->GetSubsystem<UOWBallisticsSubsystem>()->Fire(Pawn, Pawn->GetController(), Origin, Direction * FMath::Clamp(Definition->MuzzleVelocityMPS, 1.f, 2000.f) * 100.f, Definition->Damage))
    {
        --Magazine; NextShotAt = Now + FMath::Max(0.05f, Definition->ShotInterval);
        UAISense_Hearing::ReportNoiseEvent(GetWorld(), Origin, 1.f, Pawn, 5000.f, "Gunshot");
        ShotFX(Origin, Direction);
    }
}
void UOWWeaponComponent::Reload()
{
    if (!GetOwner()->HasAuthority() || !Definition || bReloading || Magazine >= Definition->MagazineSize) return;
    const UOWHealthComponent* Health = GetOwner()->FindComponentByClass<UOWHealthComponent>();
    if ((Health && Health->IsDead()) || !Inventory() || Inventory()->Count(Definition->AmmoItem) == 0) return;
    StopFire(); bReloading = true;
    GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &UOWWeaponComponent::FinishReload, FMath::Max(0.1f, Definition->ReloadSeconds), false);
}
void UOWWeaponComponent::FinishReload()
{
    if (UOWInventoryComponent* Inv = Inventory())
    {
        if (Definition)
        {
            const int32 Needed = FMath::Min(FMath::Clamp(Definition->MagazineSize, 1, 200) - Magazine, Inv->Count(Definition->AmmoItem));
            if (Needed > 0 && Inv->Remove(Definition->AmmoItem, Needed)) Magazine += Needed;
        }
    }
    bReloading = false;
}
void UOWWeaponComponent::CancelActions()
{
    StopFire(); GetWorld()->GetTimerManager().ClearTimer(ReloadTimer); bReloading = false;
}
bool UOWWeaponComponent::RestoreMagazine(int32 Value)
{
    if (!GetOwner()->HasAuthority() || Value < 0 || Value > (Definition ? FMath::Clamp(Definition->MagazineSize, 1, 200) : 0)) return false;
    CancelActions(); Magazine = Value; return true;
}
void UOWWeaponComponent::ShotFX_Implementation(FVector_NetQuantize Origin, FVector_NetQuantizeNormal Direction)
{
    if (GetWorld()->GetNetMode() != NM_DedicatedServer) { OnShot.Broadcast(Origin, Direction); OnShotFX(Origin, Direction); }
}
void UOWWeaponComponent::EndPlay(const EEndPlayReason::Type Reason) { CancelActions(); Super::EndPlay(Reason); }
void UOWWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(UOWWeaponComponent, Magazine, COND_OwnerOnly);
    DOREPLIFETIME(UOWWeaponComponent, bReloading);
}
