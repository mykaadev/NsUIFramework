#include "NsUILibrary.h"

#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "NsUISubsystem.h"

namespace
{
UNsUISubsystem* GetSubsystemFromContext(UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return nullptr;
    }

    if (ULocalPlayer* LocalPlayer = WorldContextObject->GetWorld() ? WorldContextObject->GetWorld()->GetFirstLocalPlayerFromController() : nullptr)
    {
        return LocalPlayer->GetSubsystem<UNsUISubsystem>();
    }

    return nullptr;
}
}

UUserWidget* UNsUILibrary::Push(UObject* WorldContextObject, TSubclassOf<UUserWidget> ScreenClass, ENsUILayer Layer)
{
    if (UNsUISubsystem* Subsystem = GetSubsystemFromContext(WorldContextObject))
    {
        return Subsystem->PushWidgetToLayer(ScreenClass, Layer);
    }

    return nullptr;
}

void UNsUILibrary::Pop(UObject* WorldContextObject, ENsUILayer Layer)
{
    if (UNsUISubsystem* Subsystem = GetSubsystemFromContext(WorldContextObject))
    {
        Subsystem->PopWidgetFromLayer(Layer);
    }
}

void UNsUILibrary::PopAll(UObject* WorldContextObject, ENsUILayer Layer)
{
    if (UNsUISubsystem* Subsystem = GetSubsystemFromContext(WorldContextObject))
    {
        Subsystem->PopAllWidgetsFromLayer(Layer);
    }
}

void UNsUILibrary::ToggleDebugOverlay(UObject* WorldContextObject)
{
    if (UNsUISubsystem* Subsystem = GetSubsystemFromContext(WorldContextObject))
    {
        Subsystem->ToggleDebugOverlay();
    }
}
