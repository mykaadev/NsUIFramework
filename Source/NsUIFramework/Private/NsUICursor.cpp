#include "NsUICursor.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"

void UNsUICursor::MoveToWidget(UWidget* InTarget)
{
    if (!InTarget)
    {
        SetVisibility(ESlateVisibility::Hidden);
        return;
    }

    const FGeometry Geometry = InTarget->GetCachedGeometry();
    const FVector2D LocalSize = Geometry.GetLocalSize();
    const FVector2D AbsolutePosition = Geometry.LocalToAbsolute(FVector2D::ZeroVector);

    if (UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(this))
    {
        CanvasSlot->SetPosition(AbsolutePosition);
        CanvasSlot->SetSize(LocalSize);
    }

    SetVisibility(ESlateVisibility::HitTestInvisible);
}

