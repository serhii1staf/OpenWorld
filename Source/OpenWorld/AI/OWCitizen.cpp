#include "AI/OWCitizen.h"
#include "AI/OWPopulationSubsystem.h"
#include "Character/OWHealthComponent.h"
#include "Environment/OWGameState.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/World.h"
AOWCitizenController::AOWCitizenController()
{
    Senses = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Senses")); SetPerceptionComponent(*Senses);
    UAISenseConfig_Hearing* Hearing = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing"));
    Hearing->HearingRange = 4000.f; Hearing->SetMaxAge(5.f);
    Hearing->DetectionByAffiliation.bDetectEnemies = true; Hearing->DetectionByAffiliation.bDetectFriendlies = true; Hearing->DetectionByAffiliation.bDetectNeutrals = true;
    Senses->ConfigureSense(*Hearing); Senses->OnTargetPerceptionUpdated.AddDynamic(this, &AOWCitizenController::Heard);
}
void AOWCitizenController::Heard(AActor* Actor, FAIStimulus Stimulus)
{
    if (HasAuthority() && Actor && Stimulus.WasSuccessfullySensed()) if (AOWCitizen* C = Cast<AOWCitizen>(GetPawn())) C->Flee(Stimulus.StimulusLocation);
}
AOWCitizen::AOWCitizen()
{
    bReplicates = true; AIControllerClass = AOWCitizenController::StaticClass(); AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    Health = CreateDefaultSubobject<UOWHealthComponent>(TEXT("Health"));
    GetCharacterMovement()->MaxWalkSpeed = 150.f; GetCharacterMovement()->bOrientRotationToMovement = true;
    GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
    bUseControllerRotationYaw = false;
}
void AOWCitizen::BeginPlay()
{
    Super::BeginPlay(); if (HasAuthority()) GetWorld()->GetSubsystem<UOWPopulationSubsystem>()->Register(this);
}
void AOWCitizen::Flee(FVector Danger) { Threat = Danger; FleeUntil = GetWorld()->GetTimeSeconds() + 12.0; NextDecisionAt = 0; }
void AOWCitizen::Service(double NearestSquared)
{
    AAIController* AI = Cast<AAIController>(GetController()); if (!AI || !HasAuthority()) return;
    const bool bShouldWake = !Health->IsDead() && NearestSquared < FMath::Square(bAwake ? 18000.0 : 15000.0);
    if (bShouldWake != bAwake)
    {
        bAwake = bShouldWake; if (!bAwake) AI->StopMovement();
        GetCharacterMovement()->SetComponentTickEnabled(bAwake);
        AI->SetActorTickEnabled(bAwake);
    }
    if (!bAwake || GetWorld()->GetTimeSeconds() < NextDecisionAt) return;
    NextDecisionAt = GetWorld()->GetTimeSeconds() + 3.0;
    const AOWGameState* State = GetWorld()->GetGameState<AOWGameState>();
    const bool bFlee = GetWorld()->GetTimeSeconds() < FleeUntil;
    const float Hour = State ? State->HourOfDay() : 12.f;
    FVector Goal = (Hour >= 8.f && Hour < 18.f && (!State || State->Rain < 0.7f)) ? Work : Home;
    if (bFlee) Goal = GetActorLocation() + (GetActorLocation() - Threat).GetSafeNormal2D() * 2500.f;
    GetCharacterMovement()->MaxWalkSpeed = bFlee ? 480.f : 150.f;
    if (UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
    {
        FNavLocation Projected;
        if (Nav->ProjectPointToNavigation(Goal, Projected, FVector(300,300,500))) AI->MoveToLocation(Projected.Location, 100.f);
    }
}
void AOWCitizen::EndPlay(const EEndPlayReason::Type Reason)
{
    if (HasAuthority()) GetWorld()->GetSubsystem<UOWPopulationSubsystem>()->Unregister(this);
    Super::EndPlay(Reason);
}
