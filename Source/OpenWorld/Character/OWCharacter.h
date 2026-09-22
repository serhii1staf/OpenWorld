#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OWCharacter.generated.h"
class USpringArmComponent;
class UCameraComponent;
class UOWHealthComponent;
class UOWWeaponComponent;
class UPawnNoiseEmitterComponent;
UCLASS()
class OPENWORLD_API AOWCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    AOWCharacter();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UOWHealthComponent> Health;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UOWWeaponComponent> Weapon;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<USpringArmComponent> CameraBoom;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UCameraComponent> Camera;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UPawnNoiseEmitterComponent> NoiseEmitter;
    AActor* FindInteraction() const;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
protected:
    virtual void BeginPlay() override;
private:
    UPROPERTY(ReplicatedUsing=ApplySprint) bool bSprinting = false;
    double NextInteractionAt = 0;
    void Forward(float Value);
    void Right(float Value);
    void Turn(float Value);
    void Look(float Value);
    void SprintOn(); void SprintOff(); void ToggleCrouch();
    void FireOn(); void FireOff(); void RequestReload(); void RequestInteract();
    UFUNCTION(Server, Reliable) void ServerSprint(bool bEnabled);
    UFUNCTION(Server, Reliable) void ServerFire(bool bEnabled);
    UFUNCTION(Server, Reliable) void ServerReload();
    UFUNCTION(Server, Reliable) void ServerInteract();
    UFUNCTION() void ApplySprint();
    UFUNCTION() void HealthChanged(float Value, bool bDead);
};
