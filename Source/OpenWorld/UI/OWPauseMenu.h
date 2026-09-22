#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OWPauseMenu.generated.h"
class UComboBoxString;
class USlider;
class UVerticalBox;
class UTextBlock;
UCLASS()
class OPENWORLD_API UOWPauseMenu : public UUserWidget
{
    GENERATED_BODY()
protected:
    virtual void NativeOnInitialized() override;
private:
    UPROPERTY() TObjectPtr<UComboBoxString> Preset;
    UPROPERTY() TObjectPtr<UComboBoxString> Resolution;
    UPROPERTY() TObjectPtr<UComboBoxString> Framerate;
    UPROPERTY() TObjectPtr<USlider> RenderScale;
    void Label(UVerticalBox* Box, const FText& Text, int32 Size);
    UFUNCTION() void Resume();
    UFUNCTION() void Apply();
    UFUNCTION() void Quit();
};
