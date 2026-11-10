#include "NsUIHUD.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "NsUIFunctionLibrary.h"
#include "NsUISubsystem.h"

void ANsUIHUD::BeginPlay()
{
    Super::BeginPlay();
    APlayerController* const Player = GetOwningPlayerController();
    if (Player == nullptr || !Player->IsLocalController())
    {
        return;
    }
    for (const FNsUIStartWidget& Start : StartWidgets)
    {
        if (UUserWidget* const Widget = UNsUIFunctionLibrary::OpenWidget(Player, Start.Widget, nullptr, Start.Layer))
        {
            OpenedWidgets.Add(Widget);
        }
    }
}

void ANsUIHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UNsUISubsystem* const UI = UNsUIFunctionLibrary::GetUISubsystem(GetOwningPlayerController());
    for (const TWeakObjectPtr<UUserWidget>& Widget : OpenedWidgets)
    {
        if (UI != nullptr && UI->IsWidgetOpen(Widget.Get()))
        {
            UI->CloseWidget(Widget.Get());
        }
    }
    OpenedWidgets.Reset();
    Super::EndPlay(EndPlayReason);
}

UUserWidget* ANsUIHUD::GetStartWidget(const TSubclassOf<UUserWidget> WidgetClass) const
{
    const UNsUISubsystem* const UI = UNsUIFunctionLibrary::GetUISubsystem(GetOwningPlayerController());
    for (const TWeakObjectPtr<UUserWidget>& Widget : OpenedWidgets)
    {
        if (Widget.IsValid() && WidgetClass != nullptr && Widget->IsA(WidgetClass) && UI != nullptr && UI->IsWidgetOpen(Widget.Get()))
        {
            return Widget.Get();
        }
    }
    return nullptr;
}
