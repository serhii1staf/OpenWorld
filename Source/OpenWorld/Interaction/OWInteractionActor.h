#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/OWInteractable.h"
#include "SaveGame/OWPersistentEntity.h"
#include "OWInteractionActor.generated.h"
class UStaticMeshComponent;
UCLASS()
class OPENWORLD_API AOWInteractionActor : public AActor, public IOWInteractable, public IOWPersistentEntity
{
    GENERATED_BODY()
public:
    AOWInteractionActor();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Mesh;
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly) FGuid SaveId;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText Prompt;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName MissionToStart;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName MissionEvent;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName RequiredItem;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName GrantedItem;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0")) int32 Cost = 0;
    UPROPERTY(ReplicatedUsing=StateChanged, BlueprintReadOnly, SaveGame) bool bUsed = false;
    virtual FText GetPrompt_Implementation(APawn* User) const override;
    virtual bool CanInteract_Implementation(APawn* User) const override;
    virtual void Interact_Implementation(APawn* User) override;
    virtual FGuid PersistentId() const override { return SaveId; }
    virtual FOWWorldRecord CapturePersistentState() const override;
    virtual void RestorePersistentState(const FOWWorldRecord& State) override;
    UFUNCTION(BlueprintImplementableEvent) void PresentationChanged(bool bNewUsed);
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UFUNCTION() void StateChanged();
};
