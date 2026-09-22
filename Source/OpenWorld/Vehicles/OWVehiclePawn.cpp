#include "Vehicles/OWVehiclePawn.h"
#include "Character/OWCharacter.h"
#include "Character/OWHealthComponent.h"
#include "Weapons/OWWeaponComponent.h"
#include "SaveGame/OWWorldStateSubsystem.h"
#include "ChaosVehicleMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
AOWVehiclePawn::AOWVehiclePawn()
{
    bReplicates = true; SetReplicateMovement(true);
    Health = CreateDefaultSubobject<UOWHealthComponent>(TEXT("Health"));
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(GetMesh()); CameraBoom->TargetArmLength = 600.f; CameraBoom->SocketOffset.Z = 180.f; CameraBoom->bUsePawnControlRotation = true;
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera")); Camera->SetupAttachment(CameraBoom);
}
void AOWVehiclePawn::BeginPlay()
{
    Super::BeginPlay(); Health->OnChanged.AddDynamic(this, &AOWVehiclePawn::VehicleHealthChanged);
    if (HasAuthority() && !GetWorld()->GetSubsystem<UOWWorldStateSubsystem>()->Register(this))
        UE_LOG(LogTemp, Error, TEXT("Vehicle missing unique persistent ID: %s"), *GetPathName());
}
FText AOWVehiclePawn::GetPrompt_Implementation(APawn*) const { return NSLOCTEXT("OpenWorld", "Drive", "Enter vehicle"); }
bool AOWVehiclePawn::CanInteract_Implementation(APawn* User) const
{
    const AOWCharacter* C = Cast<AOWCharacter>(User);
    return C && C->GetController() && !C->Health->IsDead() && !Driver && !Health->IsDead() && GetVelocity().Size() < 150.f && FVector::DistSquared(C->GetActorLocation(), GetActorLocation()) <= FMath::Square(400.f);
}
void AOWVehiclePawn::Interact_Implementation(APawn* User)
{
    if (!HasAuthority() || !CanInteract_Implementation(User)) return;
    Driver = CastChecked<AOWCharacter>(User);
    AController* PC = Driver->GetController(); Driver->Weapon->CancelActions();
    Driver->GetCharacterMovement()->StopMovementImmediately(); Driver->GetCharacterMovement()->DisableMovement();
    Driver->SetActorEnableCollision(false);
    Driver->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    DriverChanged(); PC->Possess(this);
}
void AOWVehiclePawn::DriverChanged()
{
    if (Driver) Driver->SetActorHiddenInGame(true);
}
void AOWVehiclePawn::RequestExit() { ServerExit(); }
void AOWVehiclePawn::ServerExit_Implementation()
{
    if (!Driver || !GetController() || GetVelocity().Size() > 150.f) return;
    const float Radius = Driver->GetCapsuleComponent()->GetScaledCapsuleRadius();
    const float Height = Driver->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    FCollisionQueryParams Params(SCENE_QUERY_STAT(OWVehicleExit), false, this); Params.AddIgnoredActor(Driver);
    FVector Safe; bool bFound = false;
    for (float Side : {-1.f, 1.f})
    {
        FVector Candidate = GetActorLocation() + GetActorRightVector() * 230.f * Side;
        FHitResult Floor;
        if (!GetWorld()->LineTraceSingleByChannel(Floor, Candidate + FVector(0,0,200), Candidate - FVector(0,0,500), ECC_Visibility, Params) || Floor.ImpactNormal.Z < 0.7f) continue;
        Candidate = Floor.ImpactPoint + FVector(0,0,Height + 5.f);
        FHitResult Path;
        const FVector PathStart = GetActorLocation() + FVector(0,0,Height);
        if (GetWorld()->SweepSingleByChannel(Path, PathStart, Candidate, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeCapsule(Radius, Height), Params)) continue;
        if (!GetWorld()->OverlapBlockingTestByChannel(Candidate, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeCapsule(Radius, Height), Params))
        { Safe = Candidate; bFound = true; break; }
    }
    if (!bFound) return;
    AOWCharacter* C = Driver; AController* PC = GetController();
    GetVehicleMovementComponent()->SetThrottleInput(0.f); GetVehicleMovementComponent()->SetBrakeInput(1.f);
    C->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    C->SetActorLocation(Safe, false, nullptr, ETeleportType::TeleportPhysics);
    C->SetActorHiddenInGame(false); C->SetActorEnableCollision(true);
    C->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    Driver = nullptr; PC->Possess(C);
}
void AOWVehiclePawn::Throttle(float Value)
{
    const bool bReverse = GetVehicleMovementComponent()->GetForwardSpeed() < -50.f;
    const bool bForward = GetVehicleMovementComponent()->GetForwardSpeed() > 50.f;
    const bool bBraking = (Value < 0 && bForward) || (Value > 0 && bReverse);
    GetVehicleMovementComponent()->SetBrakeInput(bBraking ? FMath::Abs(Value) : 0.f);
    GetVehicleMovementComponent()->SetThrottleInput(Health->IsDead() || bBraking ? 0.f : FMath::Abs(Value));
    if (!bBraking && FMath::Abs(Value) > 0.01f) GetVehicleMovementComponent()->SetTargetGear(Value < 0 ? -1 : 1, true);
}
void AOWVehiclePawn::Steering(float Value) { GetVehicleMovementComponent()->SetSteeringInput(FMath::Clamp(Value,-1.f,1.f)); }
void AOWVehiclePawn::HandbrakeOn() { GetVehicleMovementComponent()->SetHandbrakeInput(true); }
void AOWVehiclePawn::HandbrakeOff() { GetVehicleMovementComponent()->SetHandbrakeInput(false); }
void AOWVehiclePawn::Turn(float Value) { AddControllerYawInput(Value); }
void AOWVehiclePawn::Look(float Value) { AddControllerPitchInput(Value); }
void AOWVehiclePawn::VehicleHealthChanged(float, bool bDead)
{
    if (bDead) { GetVehicleMovementComponent()->SetThrottleInput(0.f); GetVehicleMovementComponent()->SetBrakeInput(1.f); }
}
void AOWVehiclePawn::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    Input->BindAxis("Forward", this, &AOWVehiclePawn::Throttle); Input->BindAxis("Right", this, &AOWVehiclePawn::Steering);
    Input->BindAxis("Turn", this, &AOWVehiclePawn::Turn); Input->BindAxis("Look", this, &AOWVehiclePawn::Look);
    Input->BindAction("Interact", IE_Pressed, this, &AOWVehiclePawn::RequestExit);
    Input->BindAction("Handbrake", IE_Pressed, this, &AOWVehiclePawn::HandbrakeOn); Input->BindAction("Handbrake", IE_Released, this, &AOWVehiclePawn::HandbrakeOff);
}
FOWWorldRecord AOWVehiclePawn::CapturePersistentState() const
{
    FOWWorldRecord R; R.Id = SaveId; R.Kind = "Vehicle"; R.Transform = GetActorTransform(); R.Health = Health->GetHealth(); return R;
}
void AOWVehiclePawn::RestorePersistentState(const FOWWorldRecord& R)
{
    if (!HasAuthority() || R.Id != SaveId || R.Kind != "Vehicle" || Driver) return;
    SetActorTransform(R.Transform, false, nullptr, ETeleportType::TeleportPhysics);
    GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector); GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    Health->Restore(R.Health);
}
void AOWVehiclePawn::EndPlay(const EEndPlayReason::Type Reason)
{
    if (HasAuthority()) GetWorld()->GetSubsystem<UOWWorldStateSubsystem>()->Unregister(this);
    Super::EndPlay(Reason);
}
void AOWVehiclePawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps); DOREPLIFETIME(AOWVehiclePawn, Driver);
}
