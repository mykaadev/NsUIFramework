#include "NsUIWidget.h"

#include "NsUISubsystem.h"
#include "Engine/LocalPlayer.h"

UNsUIWidget::UNsUIWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bFocusOnHover(true)
{
    SetIsFocusable(true);
}

void UNsUIWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
}

void UNsUIWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

    if (bFocusOnHover)
    {
        RequestFocus(EFocusCause::Mouse);
    }
}

void UNsUIWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);
}

UWidget* UNsUIWidget::GetInitialFocusWidget_Implementation() const
{
    return nullptr;
}

void UNsUIWidget::RequestFocus(EFocusCause Cause)
{
    if (!IsInteractable())
    {
        return;
    }

    if (UNsUISubsystem* Subsystem = GetOwningLocalPlayer() ? GetOwningLocalPlayer()->GetSubsystem<UNsUISubsystem>() : nullptr)
    {
        Subsystem->HandleHoverFocus(this);
    }
}
