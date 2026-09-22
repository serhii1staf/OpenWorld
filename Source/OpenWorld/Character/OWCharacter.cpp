#include "Character/OWCharacter.h"
#include "Character/OWHealthComponent.h"
#include "Weapons/OWWeaponComponent.h"
#include "Interaction/OWInteractable.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
AOWCharacter::AOWCharacter()
{
    bReplicates = true;
    bUseControllerRotationYaw = false;
    Health = CreateDefaultSubobject<UOWHealthComponent>(TEXT("Health"));
    Weapon = CreateDefaultSubobject<UOWWeaponComponent>(TEXT("Weapon"));
    NoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("Noise"));
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(GetRootComponent()); CameraBoom->TargetArmLength = 320.f; CameraBoom->bUsePawnControlRotation = true;
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera")); Camera->SetupAttachment(CameraBoom);
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
    GetCharacterMovement()->MaxWalkSpeed = 420.f;
}
void AOWCharacter::BeginPlay()
{
    Super::BeginPlay(); Health->OnChanged.AddDynamic(this, &AOWCharacter::HealthChanged);
}
void AOWCharacter::Forward(float Value)
{
    if (Controller && !Health->IsDead()) AddMovementInput(FRotationMatrix(FRotator(0, Controller->GetControlRotation().Yaw, 0)).GetUnitAxis(EAxis::X), Value);
}
void AOWCharacter::Right(float Value)
{
    if (Controller && !Health->IsDead()) AddMovementInput(FRotationMatrix(FRotator(0, Controller->GetControlRotation().Yaw, 0)).GetUnitAxis(EAxis::Y), Value);
}
void AOWCharacter::Turn(float Value) { AddControllerYawInput(Value); }
void AOWCharacter::Look(float Value) { AddControllerPitchInput(Value); }
void AOWCharacter::SprintOn() { ServerSprint(true); }
void AOWCharacter::SprintOff() { ServerSprint(false); }
void AOWCharacter::ServerSprint_Implementation(bool bEnabled) { bSprinting = bEnabled && !Health->IsDead(); ApplySprint(); }
void AOWCharacter::ApplySprint() { GetCharacterMovement()->MaxWalkSpeed = bSprinting ? 650.f : 420.f; }
void AOWCharacter::ToggleCrouch() { if (bIsCrouched) UnCrouch(); else if (!Health->IsDead()) Crouch(); }
void AOWCharacter::FireOn() { ServerFire(true); }
void AOWCharacter::FireOff() { ServerFire(false); }
void AOWCharacter::RequestReload() { ServerReload(); }
void AOWCharacter::ServerFire_Implementation(bool bEnabled) { if (bEnabled && !Health->IsDead()) Weapon->StartFire(); else Weapon->StopFire(); }
void AOWCharacter::ServerReload_Implementation() { if (!Health->IsDead()) Weapon->Reload(); }
void AOWCharacter::RequestInteract() { ServerInteract(); }
AActor* AOWCharacter::FindInteraction() const
{
    FVector Origin; FRotator Rotation; GetActorEyesViewPoint(Origin, Rotation);
    FHitResult Hit; FCollisionQueryParams Params(SCENE_QUERY_STAT(OWInteraction), false, this);
    // Trace from the pawn, not the third-person camera: walls cannot be bypassed.
    if (GetWorld()->LineTraceSingleByChannel(Hit, Origin, Origin + Rotation.Vector() * 300.f, ECC_Visibility, Params))
    {
        AActor* A = Hit.GetActor();
        if (IsValid(A) && A->Implements<UOWInteractable>()) return A;
    }
    return nullptr;
}
void AOWCharacter::ServerInteract_Implementation()
{
    if (Health->IsDead() || GetWorld()->GetTimeSeconds() < NextInteractionAt) return;
    NextInteractionAt = GetWorld()->GetTimeSeconds() + 0.2;
    if (AActor* A = FindInteraction())
        if (IOWInteractable::Execute_CanInteract(A, this)) IOWInteractable::Execute_Interact(A, this);
}
void AOWCharacter::HealthChanged(float, bool bDead)
{
    if (bDead) { GetCharacterMovement()->DisableMovement(); Weapon->CancelActions(); }
    else if (GetCharacterMovement()->MovementMode == MOVE_None) GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}
void AOWCharacter::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    Input->BindAxis("Forward", this, &AOWCharacter::Forward); Input->BindAxis("Right", this, &AOWCharacter::Right);
    Input->BindAxis("Turn", this, &AOWCharacter::Turn); Input->BindAxis("Look", this, &AOWCharacter::Look);
    Input->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump); Input->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
    Input->BindAction("Sprint", IE_Pressed, this, &AOWCharacter::SprintOn); Input->BindAction("Sprint", IE_Released, this, &AOWCharacter::SprintOff);
    Input->BindAction("Crouch", IE_Pressed, this, &AOWCharacter::ToggleCrouch);
    Input->BindAction("Interact", IE_Pressed, this, &AOWCharacter::RequestInteract);
    Input->BindAction("Fire", IE_Pressed, this, &AOWCharacter::FireOn); Input->BindAction("Fire", IE_Released, this, &AOWCharacter::FireOff);
    Input->BindAction("Reload", IE_Pressed, this, &AOWCharacter::RequestReload);
}
void AOWCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const
{
    Super::GetLifetimeReplicatedProps(Out); DOREPLIFETIME(AOWCharacter, bSprinting);
}
