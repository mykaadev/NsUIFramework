#include "NsUINavigationBehaviour.h"

#include "NsUISubsystem.h"

void UNsUINavigationBehaviour::OnEnter_Implementation() {}

void UNsUINavigationBehaviour::OnExit_Implementation() {}

void UNsUINavigationBehaviour::OnTick_Implementation(float DeltaTime) {}

void UNsUINavigationBehaviour::OnNavigate_Implementation(EUINavigation Direction)
{
    if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
    {
        Subsystem->NavigateSelectionTargets(Direction);
    }
}

void UNsUINavigationBehaviour::OnConfirm_Implementation(const FNsUIFocusTarget& Target) {}

void UNsUINavigationBehaviour::OnHover_Implementation(const FNsUIFocusTarget& Target) {}

void UNsUINavigationBehaviour::OnCancel_Implementation() {}
