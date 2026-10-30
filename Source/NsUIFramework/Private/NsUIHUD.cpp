#include "NsUIHUD.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "NsUIFunctionLibrary.h"

void ANsUIHUD::BeginPlay()
{
    Super::BeginPlay();
    APlayerController* Player = GetOwningPlayerController();
    if (!Player || !Player->IsLocalController())
    {
        return;
    }
    for (const FNsUIStartScreen& Start : StartScreens)
    {
        if (UUserWidget* Screen = UNsUIFunctionLibrary::OpenScreen(Player, Start.Screen, Start.Layer))
        {
            OpenScreens.Add(Screen);
        }
    }
}

UUserWidget* ANsUIHUD::GetStartScreen(TSubclassOf<UUserWidget> ScreenClass) const
{
    for (UUserWidget* Screen : OpenScreens)
    {
        if (Screen && ScreenClass && Screen->IsA(ScreenClass))
        {
            return Screen;
        }
    }
    return nullptr;
}
