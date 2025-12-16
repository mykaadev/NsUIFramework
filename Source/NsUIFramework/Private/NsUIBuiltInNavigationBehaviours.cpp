#include "NsUIBuiltInNavigationBehaviours.h"

#include "NsUISubsystem.h"

void UNsUIWidgetNavigationBehaviour::OnNavigate_Implementation(EUINavigation Direction)
{
    if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
    {
        Subsystem->NavigateWidgets(Direction);
    }
}

void UNsUIWidgetNavigationBehaviour::OnConfirm_Implementation()
{
    if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
    {
        Subsystem->ForwardConfirmToSlate();
    }
}

void UNsUIWidgetNavigationBehaviour::OnCancel_Implementation()
{
    if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
    {
        Subsystem->ForwardCancelToSlate();
    }
}

void UNsUIActorNavigationBehaviour::OnEnter_Implementation()
{
    FocusActorAtIndex(CurrentIndex == INDEX_NONE ? 0 : CurrentIndex);
}

void UNsUIActorNavigationBehaviour::OnTick_Implementation(float DeltaTime)
{
    if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
    {
        Subsystem->UpdateCursorToActor(GetCurrentActor());
    }
}

void UNsUIActorNavigationBehaviour::OnConfirm_Implementation()
{
    // Default actor confirm is a no-op so games can override in Blueprint/C++ as needed.
}

void UNsUIActorNavigationBehaviour::OnCancel_Implementation()
{
    // Default actor cancel is a no-op so games can override in Blueprint/C++ as needed.
}

void UNsUILocationNavigationBehaviour::OnNavigate_Implementation(EUINavigation Direction)
{
    // Location navigation is intentionally empty here; implement custom logic per experience.
}

void UNsUILocationNavigationBehaviour::OnConfirm_Implementation() {}

void UNsUILocationNavigationBehaviour::OnCancel_Implementation() {}

void UNsUIActorNavigationBehaviour::SetActors(const TArray<AActor*>& InActors)
{
    Actors.Reset();
    for (AActor* Actor : InActors)
    {
        Actors.Add(Actor);
    }

    CurrentIndex = Actors.Num() > 0 ? 0 : INDEX_NONE;
    FocusActorAtIndex(CurrentIndex);
}

void UNsUIActorNavigationBehaviour::ClearActors()
{
    Actors.Reset();
    CurrentIndex = INDEX_NONE;

    if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
    {
        Subsystem->UpdateCursorToActor(nullptr);
    }
}

void UNsUIActorNavigationBehaviour::FocusActorAtIndex(int32 Index)
{
    if (!Actors.IsValidIndex(Index))
    {
        return;
    }

    CurrentIndex = Index;

    if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
    {
        Subsystem->UpdateCursorToActor(GetCurrentActor());
    }
}

void UNsUIActorNavigationBehaviour::OnNavigate_Implementation(EUINavigation Direction)
{
    if (Actors.Num() == 0)
    {
        return;
    }

    int32 NextIndex = CurrentIndex;
    switch (Direction)
    {
    case EUINavigation::Right:
    case EUINavigation::Down:
    case EUINavigation::Next:
        NextIndex = (CurrentIndex + 1 + Actors.Num()) % Actors.Num();
        break;
    case EUINavigation::Left:
    case EUINavigation::Up:
    case EUINavigation::Previous:
        NextIndex = (CurrentIndex - 1 + Actors.Num()) % Actors.Num();
        break;
    default:
        break;
    }

    FocusActorAtIndex(NextIndex);
}

AActor* UNsUIActorNavigationBehaviour::GetCurrentActor() const
{
    return Actors.IsValidIndex(CurrentIndex) ? Actors[CurrentIndex].Get() : nullptr;
}
