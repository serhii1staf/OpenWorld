#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "OWInteractable.generated.h"
class APawn;
UINTERFACE(BlueprintType)
class OPENWORLD_API UOWInteractable : public UInterface { GENERATED_BODY() };
class OPENWORLD_API IOWInteractable
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable) FText GetPrompt(APawn* User) const;
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable) bool CanInteract(APawn* User) const;
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable) void Interact(APawn* User);
};
