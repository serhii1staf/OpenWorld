#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "OWPersistentEntity.generated.h"
USTRUCT(BlueprintType)
struct FOWWorldRecord
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame) FGuid Id;
    UPROPERTY(BlueprintReadWrite, SaveGame) FName Kind;
    UPROPERTY(BlueprintReadWrite, SaveGame) FTransform Transform;
    UPROPERTY(BlueprintReadWrite, SaveGame) bool bUsed = false;
    UPROPERTY(BlueprintReadWrite, SaveGame) float Health = 100.f;
};
UINTERFACE()
class OPENWORLD_API UOWPersistentEntity : public UInterface { GENERATED_BODY() };
class OPENWORLD_API IOWPersistentEntity
{
    GENERATED_BODY()
public:
    virtual FGuid PersistentId() const = 0;
    virtual FOWWorldRecord CapturePersistentState() const = 0;
    virtual void RestorePersistentState(const FOWWorldRecord& State) = 0;
};
