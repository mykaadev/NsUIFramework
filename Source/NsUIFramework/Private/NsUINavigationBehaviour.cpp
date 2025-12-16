#include "NsUINavigationBehaviour.h"

#include "NsUISubsystem.h"

void UNsUINavigationBehaviour::OnEnter_Implementation() {}

void UNsUINavigationBehaviour::OnExit_Implementation() {}

void UNsUINavigationBehaviour::OnTick_Implementation(float DeltaTime) {}

void UNsUINavigationBehaviour::OnNavigate_Implementation(EUINavigation Direction)
{
    if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
    {
        Subsystem->NavigateWidgets(Direction);
    }
}

void UNsUINavigationBehaviour::OnConfirm_Implementation() {}

void UNsUINavigationBehaviour::OnCancel_Implementation() {}
