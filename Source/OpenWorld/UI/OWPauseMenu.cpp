#include "UI/OWPauseMenu.h"
#include "Core/OWPlayerController.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
void UOWPauseMenu::Label(UVerticalBox* Box,const FText& Text,int32 Size)
{
    UTextBlock* L = WidgetTree->ConstructWidget<UTextBlock>(); L->SetText(Text);
    FSlateFontInfo Font = L->GetFont(); Font.Size = Size; L->SetFont(Font); Box->AddChildToVerticalBox(L);
}
void UOWPauseMenu::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    UBorder* Back = WidgetTree->ConstructWidget<UBorder>(); Back->SetBrushColor(FLinearColor(0.015f,0.02f,0.025f,0.97f));
    Back->SetPadding(FMargin(64.f)); WidgetTree->RootWidget = Back;
    UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(); Back->SetContent(Box);
    Label(Box,NSLOCTEXT("OpenWorld","PauseTitle","OPENWORLD / PAUSED"),32);
    Label(Box,NSLOCTEXT("OpenWorld","Preset","Quality preset"),18);
    Preset = WidgetTree->ConstructWidget<UComboBoxString>();
    for (const TCHAR* Name : {TEXT("Low"),TEXT("Medium"),TEXT("High"),TEXT("Epic")}) Preset->AddOption(Name);
    UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
    Preset->SetSelectedIndex(FMath::Clamp(Settings->GetOverallScalabilityLevel(),0,3)); Box->AddChildToVerticalBox(Preset);
    Label(Box,NSLOCTEXT("OpenWorld","Resolution","Window resolution"),18);
    Resolution = WidgetTree->ConstructWidget<UComboBoxString>();
    for (const TCHAR* Name : {TEXT("1280 x 720"),TEXT("1920 x 1080"),TEXT("2560 x 1440"),TEXT("3840 x 2160")}) Resolution->AddOption(Name);
    const FIntPoint Current = Settings->GetScreenResolution();
    Resolution->SetSelectedIndex(Current.X >= 3840 ? 3 : Current.X >= 2560 ? 2 : Current.X >= 1920 ? 1 : 0); Box->AddChildToVerticalBox(Resolution);
    Label(Box,NSLOCTEXT("OpenWorld","Scale","Internal render scale (50-100%, TSR)"),18);
    RenderScale = WidgetTree->ConstructWidget<USlider>(); RenderScale->SetMinValue(50.f); RenderScale->SetMaxValue(100.f);
    float Normalized,Value,Minimum,Maximum; Settings->GetResolutionScaleInformationEx(Normalized,Value,Minimum,Maximum);
    RenderScale->SetValue(FMath::Clamp(Value,50.f,100.f)); Box->AddChildToVerticalBox(RenderScale);
    Label(Box,NSLOCTEXT("OpenWorld","Framerate","Frame rate limit"),18);
    Framerate = WidgetTree->ConstructWidget<UComboBoxString>();
    for (const TCHAR* Name : {TEXT("30"),TEXT("60"),TEXT("120"),TEXT("Unlimited")}) Framerate->AddOption(Name);
    const float Limit = Settings->GetFrameRateLimit(); Framerate->SetSelectedIndex(Limit <= 0 ? 3 : Limit <= 30 ? 0 : Limit <= 60 ? 1 : 2); Box->AddChildToVerticalBox(Framerate);
    auto Button = [this,Box](const FText& Text)
    {
        UButton* B = WidgetTree->ConstructWidget<UButton>(); UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>(); T->SetText(Text); B->SetContent(T); Box->AddChildToVerticalBox(B); return B;
    };
    Button(NSLOCTEXT("OpenWorld","Apply","Apply graphics"))->OnClicked.AddDynamic(this,&UOWPauseMenu::Apply);
    Button(NSLOCTEXT("OpenWorld","Resume","Resume"))->OnClicked.AddDynamic(this,&UOWPauseMenu::Resume);
    Button(NSLOCTEXT("OpenWorld","Quit","Quit to desktop"))->OnClicked.AddDynamic(this,&UOWPauseMenu::Quit);
}
void UOWPauseMenu::Apply()
{
    UGameUserSettings* S = UGameUserSettings::GetGameUserSettings();
    static const FIntPoint Sizes[] = {FIntPoint(1280,720),FIntPoint(1920,1080),FIntPoint(2560,1440),FIntPoint(3840,2160)};
    static const float Rates[] = {30.f,60.f,120.f,0.f};
    S->SetOverallScalabilityLevel(FMath::Clamp(Preset->GetSelectedIndex(),0,3));
    S->SetScreenResolution(Sizes[FMath::Clamp(Resolution->GetSelectedIndex(),0,3)]); S->SetFullscreenMode(EWindowMode::Windowed);
    S->SetResolutionScaleValueEx(RenderScale->GetValue()); S->SetFrameRateLimit(Rates[FMath::Clamp(Framerate->GetSelectedIndex(),0,3)]);
    S->ApplySettings(false); S->SaveSettings();
}
void UOWPauseMenu::Resume() { if (AOWPlayerController* PC = Cast<AOWPlayerController>(GetOwningPlayer())) PC->TogglePause(); }
void UOWPauseMenu::Quit() { UKismetSystemLibrary::QuitGame(this,GetOwningPlayer(),EQuitPreference::Quit,false); }
