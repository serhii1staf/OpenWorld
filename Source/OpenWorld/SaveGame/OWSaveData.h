#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Inventory/OWInventoryComponent.h"
#include "Missions/OWMissionComponent.h"
#include "SaveGame/OWPersistentEntity.h"
#include "Environment/OWGameState.h"
#include "OWSaveData.generated.h"
UCLASS()
class OPENWORLD_API UOWSaveData : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame) int32 Schema = 1;
    UPROPERTY(SaveGame) int64 Generation = 0;
    UPROPERTY(SaveGame) FString Map;
    UPROPERTY(SaveGame) FTransform PlayerTransform;
    UPROPERTY(SaveGame) float Health = 100.f;
    UPROPERTY(SaveGame) int32 Magazine = 0;
    UPROPERTY(SaveGame) int64 Money = 0;
    UPROPERTY(SaveGame) TArray<FOWItemStack> Items;
    UPROPERTY(SaveGame) TArray<FOWMissionProgress> Missions;
    UPROPERTY(SaveGame) TArray<FOWWorldRecord> World;
    UPROPERTY(SaveGame) double WorldHours = 8.0;
    UPROPERTY(SaveGame) float Wetness = 0.f;
    UPROPERTY(SaveGame) float Rain = 0.f;
    UPROPERTY(SaveGame) float Snow = 0.f;
    UPROPERTY(SaveGame) EOWWeather Weather = EOWWeather::Clear;
};
