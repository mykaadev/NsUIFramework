#include "NsUILibrary.h"

#include "NsUISubsystem.h"

UUserWidget* UNsUILibrary::Push(TSubclassOf<UUserWidget> ScreenClass, ENsUILayer Layer)
{
    if (UNsUISubsystem* Subsystem = UNsUISubsystem::GetSubsystem())
    {
        return Subsystem->PushWidgetToLayer(ScreenClass, Layer);
    }

    return nullptr;
}

void UNsUILibrary::Pop(ENsUILayer Layer)
{
    if (UNsUISubsystem* Subsystem = UNsUISubsystem::GetSubsystem())
    {
        Subsystem->PopWidgetFromLayer(Layer);
    }
}

void UNsUILibrary::PopAll(ENsUILayer Layer)
{
    if (UNsUISubsystem* Subsystem = UNsUISubsystem::GetSubsystem())
    {
        Subsystem->PopAllWidgetsFromLayer(Layer);
    }
}

void UNsUILibrary::ToggleDebugOverlay()
{
    if (UNsUISubsystem* Subsystem = UNsUISubsystem::GetSubsystem())
    {
        Subsystem->ToggleDebugOverlay();
    }
}
