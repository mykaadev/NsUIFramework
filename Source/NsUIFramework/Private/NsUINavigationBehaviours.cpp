#include "NsUINavigationBehaviours.h"

#include "NsUISubsystem.h"

void UNsUIWidgetNavigationBehaviour::OnNavigate_Implementation(EUINavigation Direction)
{
    if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
    {
        Subsystem->NavigateWidgets(Direction);
    }
}

void UNsUIWidgetNavigationBehaviour::OnConfirm_Implementation(const FNsUIFocusTarget& Target)
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

void UNsUIActorNavigationBehaviour::OnNavigate_Implementation(EUINavigation Direction)
{
    if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
    {
        Subsystem->NavigateTargetsByKind(Direction, ENsUIFocusKind::Actor);
    }
}

void UNsUILocationNavigationBehaviour::OnNavigate_Implementation(EUINavigation Direction)
{
    if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
    {
        Subsystem->NavigateTargetsByKind(Direction, ENsUIFocusKind::Location);
    }
}

