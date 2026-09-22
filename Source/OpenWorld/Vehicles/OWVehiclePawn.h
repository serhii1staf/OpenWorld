#pragma once
#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "Interaction/OWInteractable.h"
#include "SaveGame/OWPersistentEntity.h"
#include "OWVehiclePawn.generated.h"
class AOWCharacter;
class UOWHealthComponent;
class USpringArmComponent;
class UCameraComponent;
UCLASS()
class OPENWORLD_API AOWVehiclePawn : public AWheeledVehiclePawn, public IOWInteractable, public IOWPersistentEntity
{
    GENERATED_BODY()
public:
    AOWVehiclePawn();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UOWHealthComponent> Health;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USpringArmComponent> CameraBoom;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Camera;
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly) FGuid SaveId;
    UPROPERTY(ReplicatedUsing=DriverChanged, BlueprintReadOnly) TObjectPtr<AOWCharacter> Driver;
    virtual FText GetPrompt_Implementation(APawn* User) const override;
    virtual bool CanInteract_Implementation(APawn* User) const override;
    virtual void Interact_Implementation(APawn* User) override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
    virtual FGuid PersistentId() const override { return SaveId; }
    virtual FOWWorldRecord CapturePersistentState() const override;
    virtual void RestorePersistentState(const FOWWorldRecord& State) override;
    UFUNCTION(Server, Reliable) void ServerExit();
    void RequestExit();
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    void Throttle(float Value); void Steering(float Value);
    void HandbrakeOn(); void HandbrakeOff();
    void Turn(float Value); void Look(float Value);
    UFUNCTION() void DriverChanged();
    UFUNCTION() void VehicleHealthChanged(float Value, bool bDead);
};
