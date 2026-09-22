#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OWAmbientZone.generated.h"
class USphereComponent;
class UAudioComponent;
class USoundBase;
struct FStreamableHandle;
UCLASS()
class OPENWORLD_API AOWAmbientZone : public AActor
{
    GENERATED_BODY()
public:
    AOWAmbientZone();
    UPROPERTY(VisibleAnywhere) TObjectPtr<USphereComponent> Bounds;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UAudioComponent> Audio;
    UPROPERTY(EditAnywhere) TSoftObjectPtr<USoundBase> AmbientSound;
    UPROPERTY(EditAnywhere) float FadeSeconds = 2.f;
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    TSharedPtr<FStreamableHandle> Loading;
    TSet<TWeakObjectPtr<AActor>> Listeners;
    UFUNCTION() void Enter(UPrimitiveComponent* Component,AActor* Other,UPrimitiveComponent* OtherComponent,int32 BodyIndex,bool bSweep,const FHitResult& Hit);
    UFUNCTION() void Exit(UPrimitiveComponent* Component,AActor* Other,UPrimitiveComponent* OtherComponent,int32 BodyIndex);
};
