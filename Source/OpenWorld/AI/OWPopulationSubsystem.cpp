#include "AI/OWPopulationSubsystem.h"
#include "AI/OWCitizen.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
void UOWPopulationSubsystem::OnWorldBeginPlay(UWorld& World)
{
    Super::OnWorldBeginPlay(World);
    if (World.GetNetMode() != NM_Client) World.GetTimerManager().SetTimer(Timer, this, &UOWPopulationSubsystem::Service, 0.2f, true);
}
void UOWPopulationSubsystem::Register(AOWCitizen* C) { if (IsValid(C)) Citizens.AddUnique(C); }
void UOWPopulationSubsystem::Unregister(AOWCitizen* C) { Citizens.Remove(C); }
void UOWPopulationSubsystem::Service()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(OW_Population);
    TArray<FVector, TInlineAllocator<8>> Players;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        if (const APlayerController* PC = It->Get()) if (const APawn* P = PC->GetPawn()) Players.Add(P->GetActorLocation());
    const int32 Budget = FMath::Min(16, Citizens.Num());
    for (int32 I = 0; I < Budget && !Citizens.IsEmpty(); ++I)
    {
        Cursor %= Citizens.Num();
        AOWCitizen* C = Citizens[Cursor].Get();
        if (!C) { Citizens.RemoveAtSwap(Cursor); continue; }
        ++Cursor;
        double Nearest = TNumericLimits<double>::Max();
        for (FVector P : Players) Nearest = FMath::Min(Nearest, FVector::DistSquared(P, C->GetActorLocation()));
        C->Service(Nearest);
    }
}
void UOWPopulationSubsystem::Deinitialize()
{
    if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(Timer);
    Citizens.Reset(); Super::Deinitialize();
}
