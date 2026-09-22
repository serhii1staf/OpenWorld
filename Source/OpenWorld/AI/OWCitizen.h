#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "OWCitizen.generated.h"
class UOWHealthComponent;
class UAIPerceptionComponent;
UCLASS()
class OPENWORLD_API AOWCitizenController : public AAIController
{
    GENERATED_BODY()
public:
    AOWCitizenController();
private:
    UPROPERTY() TObjectPtr<UAIPerceptionComponent> Senses;
    UFUNCTION() void Heard(AActor* Actor, FAIStimulus Stimulus);
};
UCLASS()
class OPENWORLD_API AOWCitizen : public ACharacter
{
    GENERATED_BODY()
public:
    AOWCitizen();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UOWHealthComponent> Health;
    UPROPERTY(EditInstanceOnly) FVector Home;
    UPROPERTY(EditInstanceOnly) FVector Work;
    void Service(double NearestPlayerSquared);
    void Flee(FVector Danger);
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    bool bAwake = true;
    double FleeUntil = 0;
    double NextDecisionAt = 0;
    FVector Threat = FVector::ZeroVector;
};
