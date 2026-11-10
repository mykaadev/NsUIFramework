#include "Popup/NsUIWorldPopupWidget.h"

#include "Animation/WidgetAnimation.h"
#include "Components/TextBlock.h"

UNsUIWorldPopupWidget::UNsUIWorldPopupWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Behavior defaults. The look and the motion (rise, stack, order, alignment) start neutral, and the Blueprint sets them
    Lifetime = 1.2f;
    RiseSpeed = 0.f;
    MaxDistance = 3000.f;
    bHideWhenBlocked = true;
    MaxActive = 4;
    StackHeight = 0.f;
    ViewportZOrder = 0;
    ScreenAlignment = FVector2D::ZeroVector;
}

void UNsUIWorldPopupWidget::StartPopup(const FText& Text, const FLinearColor& Color)
{
    if (PopupText != nullptr)
    {
        PopupText->SetText(Text);
        PopupText->SetColorAndOpacity(FSlateColor(Color));
    }
    if (Appear != nullptr)
    {
        PlayAnimation(Appear);
    }
    OnPopupStarted(Text, Color);
}

void UNsUIWorldPopupWidget::PlayDisappear()
{
    if (Disappear != nullptr)
    {
        PlayAnimation(Disappear);
    }
}

float UNsUIWorldPopupWidget::GetDisappearSeconds() const
{
    return Disappear != nullptr ? Disappear->GetEndTime() : 0.f;
}
