#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OWMissionDefinition.generated.h"
USTRUCT(BlueprintType)
struct FOWObjective
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText Description;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName Event;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1")) int32 Required = 1;
};
UCLASS(BlueprintType)
class OPENWORLD_API UOWMissionDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName MissionId;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText Title;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FName> Prerequisites;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FName> Excludes;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FOWObjective> Objectives;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0")) int32 Reward = 100;
};
