#include "NsUILibrary.h"

#include "NsUISubsystem.h"
#include "NsUISelectionBehavior.h"
#include "Engine/AssetManager.h"

UUserWidget* UNsUILibrary::Push(TSubclassOf<UUserWidget> ScreenClass, ENsUILayer Layer)
{
    if (UNsUISubsystem* Subsystem = UNsUISubsystem::GetSubsystem())
    {
        return Subsystem->PushWidgetToLayer(ScreenClass, Layer);
    }

    return nullptr;
}

UUserWidget* UNsUILibrary::Push(TSoftClassPtr<UUserWidget> ScreenClass, ENsUILayer Layer)
{
    if (UNsUISubsystem* Subsystem = UNsUISubsystem::GetSubsystem())
    {
        if (ScreenClass.IsNull())
        {
            return nullptr;
        }

        TSubclassOf<UUserWidget> LoadedClass = ScreenClass.LoadSynchronous();
        if (!LoadedClass)
        {
            return nullptr;
        }

        return Subsystem->PushWidgetToLayer(LoadedClass, Layer);
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

void UNsUILibrary::SetSelectionBehavior(UNsUISelectionBehavior* Behavior)
{
    if (UNsUISubsystem* Subsystem = UNsUISubsystem::GetSubsystem())
    {
        Subsystem->SetSelectionBehavior(Behavior);
    }
}

void UNsUILibrary::BeginSelectionSession(UNsUISelectionBehavior* Behavior, const TArray<AActor*>& Actors, const TArray<FVector>& Locations)
{
    if (UNsUISubsystem* Subsystem = UNsUISubsystem::GetSubsystem())
    {
        Subsystem->BeginSelectionSession(Behavior, Actors, Locations);
    }
}

void UNsUILibrary::EndSelectionSession()
{
    if (UNsUISubsystem* Subsystem = UNsUISubsystem::GetSubsystem())
    {
        Subsystem->EndSelectionSession();
    }
}

void UNsUILibrary::FocusNextTarget()
{
    if (UNsUISubsystem* Subsystem = UNsUISubsystem::GetSubsystem())
    {
        Subsystem->FocusNextTarget();
    }
}

void UNsUILibrary::FocusPreviousTarget()
{
    if (UNsUISubsystem* Subsystem = UNsUISubsystem::GetSubsystem())
    {
        Subsystem->FocusPreviousTarget();
    }
}
