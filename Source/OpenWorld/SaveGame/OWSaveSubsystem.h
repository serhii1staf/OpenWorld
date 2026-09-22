#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OWSaveSubsystem.generated.h"
class UOWSaveData;
class USaveGame;
class AOWCharacter;
class AOWPlayerState;
class AOWGameState;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOWSaveResult, bool, bSuccess, FText, Message);
UCLASS()
class OPENWORLD_API UOWSaveSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) void Save();
    UFUNCTION(BlueprintCallable) void Load();
    UFUNCTION(BlueprintPure) bool IsBusy() const { return bBusy; }
    UPROPERTY(BlueprintAssignable) FOWSaveResult OnResult;
private:
    UPROPERTY() TObjectPtr<UOWSaveData> Pending;
    UPROPERTY() TObjectPtr<UOWSaveData> SlotA;
    UPROPERTY() TObjectPtr<UOWSaveData> SlotB;
    TWeakObjectPtr<AOWCharacter> RequestCharacter;
    bool bBusy = false;
    bool bSaving = false;
    int32 ReadsRemaining = 0;
    void BeginRead();
    void ReadDone(const FString& Slot, int32 User, USaveGame* Data);
    void WriteDone(const FString& Slot, int32 User, bool bSuccess);
    void Finish(bool bSuccess, const FText& Message);
    bool Context(AOWCharacter*& Character, AOWPlayerState*& Player, AOWGameState*& State) const;
    bool Validate(const UOWSaveData* Data) const;
    bool Apply(UOWSaveData* Data);
};
