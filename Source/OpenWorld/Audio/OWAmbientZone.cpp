#include "Audio/OWAmbientZone.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Sound/SoundBase.h"
AOWAmbientZone::AOWAmbientZone()
{
    PrimaryActorTick.bCanEverTick = false;
    Bounds = CreateDefaultSubobject<USphereComponent>(TEXT("Bounds")); SetRootComponent(Bounds);
    Bounds->SetSphereRadius(3000.f); Bounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Bounds->SetCollisionResponseToAllChannels(ECR_Ignore); Bounds->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
    Audio = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio")); Audio->SetupAttachment(Bounds); Audio->bAutoActivate = false;
}
void AOWAmbientZone::BeginPlay()
{
    Super::BeginPlay();
    Bounds->OnComponentBeginOverlap.AddDynamic(this,&AOWAmbientZone::Enter);
    Bounds->OnComponentEndOverlap.AddDynamic(this,&AOWAmbientZone::Exit);
}
void AOWAmbientZone::Enter(UPrimitiveComponent*,AActor* Other,UPrimitiveComponent*,int32,bool,const FHitResult&)
{
    const APawn* P = Cast<APawn>(Other);
    if (!P || !P->IsLocallyControlled() || GetNetMode() == NM_DedicatedServer || AmbientSound.IsNull()) return;
    Listeners.Add(Other);
    if (Loading || Audio->IsPlaying()) return;
    TWeakObjectPtr<AOWAmbientZone> WeakThis(this);
    Loading = UAssetManager::GetStreamableManager().RequestAsyncLoad(AmbientSound.ToSoftObjectPath(),FStreamableDelegate::CreateLambda([WeakThis]()
    {
        if (AOWAmbientZone* Zone = WeakThis.Get())
        {
            if (!Zone->Listeners.IsEmpty() && Zone->AmbientSound.Get()) { Zone->Audio->SetSound(Zone->AmbientSound.Get()); Zone->Audio->FadeIn(FMath::Max(0.01f,Zone->FadeSeconds)); }
            Zone->Loading.Reset();
        }
    }));
}
void AOWAmbientZone::Exit(UPrimitiveComponent*,AActor* Other,UPrimitiveComponent*,int32)
{
    Listeners.Remove(Other);
    if (Listeners.IsEmpty())
    {
        if (Loading) { Loading->CancelHandle(); Loading.Reset(); }
        Audio->FadeOut(FMath::Max(0.01f,FadeSeconds),0.f);
    }
}
void AOWAmbientZone::EndPlay(const EEndPlayReason::Type Reason)
{
    if (Loading) { Loading->CancelHandle(); Loading.Reset(); }
    Audio->Stop(); Super::EndPlay(Reason);
}
