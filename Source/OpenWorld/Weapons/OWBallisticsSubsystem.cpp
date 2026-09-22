#include "Weapons/OWBallisticsSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
void UOWBallisticsSubsystem::Initialize(FSubsystemCollectionBase& C)
{
    Super::Initialize(C); Bullets.Reserve(512);
}
TStatId UOWBallisticsSubsystem::GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(OWBallistics, STATGROUP_Tickables); }
bool UOWBallisticsSubsystem::IsTickable() const
{
    return GetWorld() && GetWorld()->IsGameWorld() && GetWorld()->GetNetMode() != NM_Client && !Bullets.IsEmpty();
}
bool UOWBallisticsSubsystem::Fire(AActor* Source, AController* Controller, FVector Origin, FVector Velocity, float Damage)
{
    if (!Source || !Source->HasAuthority() || Bullets.Num() >= 512 || Origin.ContainsNaN() || Velocity.ContainsNaN() || !FMath::IsFinite(Damage) || Damage <= 0.f) return false;
    FOWBullet B; B.Source = Source; B.Controller = Controller; B.Position = Origin; B.Velocity = Velocity; B.Damage = Damage;
    Bullets.Add(B); return true;
}
void UOWBallisticsSubsystem::Tick(float Delta)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(OW_Ballistics);
    constexpr float FixedStep = 1.f / 120.f;
    // Bound catch-up cost after a hitch; no unbounded spiral of physics work.
    Accumulator = FMath::Min(Accumulator + Delta, FixedStep * 8.f);
    while (Accumulator >= FixedStep) { Step(FixedStep); Accumulator -= FixedStep; }
}
void UOWBallisticsSubsystem::Step(float Delta)
{
    for (int32 I = Bullets.Num() - 1; I >= 0; --I)
    {
        FOWBullet& B = Bullets[I]; B.Age += Delta;
        if (!B.Source.IsValid() || B.Age >= 4.f) { Bullets.RemoveAtSwap(I, EAllowShrinking::No); continue; }
        FVector NewVelocity = B.Velocity + FVector(0, 0, GetWorld()->GetGravityZ()) * Delta;
        const FVector End = B.Position + (B.Velocity + NewVelocity) * (0.5f * Delta);
        FCollisionQueryParams Params(SCENE_QUERY_STAT(OWBullet), true, B.Source.Get());
        Params.bReturnPhysicalMaterial = true;
        FHitResult Hit;
        if (GetWorld()->SweepSingleByChannel(Hit, B.Position, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(0.5f), Params))
        {
            UGameplayStatics::ApplyPointDamage(Hit.GetActor(), B.Damage, B.Velocity.GetSafeNormal(), Hit, B.Controller.Get(), B.Source.Get(), nullptr);
            OnImpact.Broadcast(Hit);
            const bool bMetal = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get()) == SurfaceType2;
            const FVector Dir = B.Velocity.GetSafeNormal();
            if (bMetal && B.Bounces < 2 && FMath::Abs(FVector::DotProduct(Dir, Hit.ImpactNormal)) < 0.25f)
            {
                ++B.Bounces; B.Damage *= 0.45f;
                B.Velocity = FMath::GetReflectionVector(B.Velocity, Hit.ImpactNormal) * 0.55f;
                B.Position = Hit.ImpactPoint + Hit.ImpactNormal * 2.f;
                continue;
            }
            Bullets.RemoveAtSwap(I, EAllowShrinking::No);
        }
        else { B.Position = End; B.Velocity = NewVelocity; }
    }
}
