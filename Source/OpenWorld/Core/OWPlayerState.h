#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "OWPlayerState.generated.h"
class UOWInventoryComponent;
class UOWMissionComponent;
UCLASS()
class OPENWORLD_API AOWPlayerState : public APlayerState
{
    GENERATED_BODY()
public:
    AOWPlayerState();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UOWInventoryComponent> Inventory;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UOWMissionComponent> Missions;
    virtual void CopyProperties(APlayerState* Other) override;
};
