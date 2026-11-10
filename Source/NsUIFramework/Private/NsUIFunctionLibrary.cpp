#include "NsUIFunctionLibrary.h"

#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "NsUISubsystem.h"

UNsUISubsystem* UNsUIFunctionLibrary::GetUISubsystem(const APlayerController* const Player)
{
    const ULocalPlayer* const LocalPlayer = Player != nullptr ? Player->GetLocalPlayer() : nullptr;
    return LocalPlayer != nullptr ? LocalPlayer->GetSubsystem<UNsUISubsystem>() : nullptr;
}

UUserWidget* UNsUIFunctionLibrary::OpenWidget(const APlayerController* const Player, const TSubclassOf<UUserWidget> WidgetClass, UObject* const Context, const ENsUILayer Layer)
{
    UNsUISubsystem* const UI = GetUISubsystem(Player);
    return UI != nullptr ? UI->OpenWidget(WidgetClass, Context, Layer) : nullptr;
}

UUserWidget* UNsUIFunctionLibrary::ToggleWidget(const APlayerController* const Player, const TSubclassOf<UUserWidget> WidgetClass, UObject* const Context, const ENsUILayer Layer)
{
    UNsUISubsystem* const UI = GetUISubsystem(Player);
    return UI != nullptr ? UI->ToggleWidget(WidgetClass, Context, Layer) : nullptr;
}

void UNsUIFunctionLibrary::CloseWidget(UUserWidget* const Widget)
{
    UNsUISubsystem* const UI = Widget != nullptr ? GetUISubsystem(Widget->GetOwningPlayer()) : nullptr;
    if (UI != nullptr)
    {
        UI->CloseWidget(Widget);
    }
}
