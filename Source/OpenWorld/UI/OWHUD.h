#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "OWHUD.generated.h"
UCLASS()
class OPENWORLD_API AOWHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
private:
    TWeakObjectPtr<AActor> CachedInteraction;
    double NextScanAt = 0;
};
