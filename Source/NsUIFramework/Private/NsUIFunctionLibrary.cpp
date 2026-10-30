#include "NsUIFunctionLibrary.h"

#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "NsUISubsystem.h"

UNsUISubsystem* UNsUIFunctionLibrary::GetUISubsystem(const APlayerController* Player)
{
    const ULocalPlayer* LocalPlayer = Player ? Player->GetLocalPlayer() : nullptr;
    return LocalPlayer ? LocalPlayer->GetSubsystem<UNsUISubsystem>() : nullptr;
}

UUserWidget* UNsUIFunctionLibrary::OpenScreen(APlayerController* Player, TSubclassOf<UUserWidget> ScreenClass, ENsUILayer Layer, UObject* Context)
{
    UNsUISubsystem* Host = GetUISubsystem(Player);
    return Host ? Host->OpenScreen(ScreenClass, Context, Layer) : nullptr;
}

UUserWidget* UNsUIFunctionLibrary::ToggleScreen(APlayerController* Player, TSubclassOf<UUserWidget> ScreenClass, ENsUILayer Layer)
{
    UNsUISubsystem* Host = GetUISubsystem(Player);
    if (!Host)
    {
        return nullptr;
    }
    if (UUserWidget* Open = Host->FindScreen(ScreenClass))
    {
        Host->CloseScreen(Open);
        return nullptr;
    }
    return Host->OpenScreen(ScreenClass, nullptr, Layer);
}

void UNsUIFunctionLibrary::CloseScreen(UUserWidget* Screen)
{
    const ULocalPlayer* LocalPlayer = Screen ? Screen->GetOwningLocalPlayer() : nullptr;
    if (UNsUISubsystem* Host = LocalPlayer ? LocalPlayer->GetSubsystem<UNsUISubsystem>() : nullptr)
    {
        Host->CloseScreen(Screen);
    }
}
