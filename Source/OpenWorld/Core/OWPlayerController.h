#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OWPlayerController.generated.h"
class UOWPauseMenu;
UCLASS()
class OPENWORLD_API AOWPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) void TogglePause();
    UPROPERTY(BlueprintReadOnly) FText LastStatus;
protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
private:
    UPROPERTY() TObjectPtr<UOWPauseMenu> PauseMenu;
    void Save(); void Load();
    UFUNCTION() void SaveResult(bool bSuccess, FText Message);
};
