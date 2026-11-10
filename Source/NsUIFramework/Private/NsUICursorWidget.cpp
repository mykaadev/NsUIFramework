#include "NsUICursorWidget.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Selection/NsUISelectionMode.h"
#include "Widgets/SWidget.h"

UNsUICursorWidget::UNsUICursorWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    AnchorInTarget = FVector2D::ZeroVector;
    CursorAlignment = FVector2D::ZeroVector;
    LastPosition = FVector2D::ZeroVector;
    bShowing = false;
}

void UNsUICursorWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetAlignmentInViewport(CursorAlignment);
    // The cursor sits over the UI and must never take a click, so code sets this and not the Blueprint
    SetVisibility(bShowing ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UNsUICursorWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    // The target can move without a focus change, for example in a list that scrolls, so the cursor reads it while it shows
    FVector2D Position;
    if (bShowing && GetTargetPosition(Position) && !Position.Equals(LastPosition, 0.5))
    {
        LastPosition = Position;
        MoveCursor(Position, false);
    }
}

void UNsUICursorWidget::FollowSlateWidget(const TSharedPtr<SWidget>& Target)
{
    TargetWidget = Target;
    TargetMode.Reset();
    FVector2D Position;
    if (!GetTargetPosition(Position))
    {
        HideCursor();
        return;
    }
    const bool bInstant = !bShowing;
    bShowing = true;
    SetVisibility(ESlateVisibility::HitTestInvisible);
    LastPosition = Position;
    MoveCursor(Position, bInstant);
}

void UNsUICursorWidget::FollowSelectionMode(UNsUISelectionMode* const Mode)
{
    TargetWidget.Reset();
    TargetMode = Mode;
    FVector2D Position;
    if (!GetTargetPosition(Position))
    {
        HideCursor();
        return;
    }
    const bool bInstant = !bShowing;
    bShowing = true;
    SetVisibility(ESlateVisibility::HitTestInvisible);
    LastPosition = Position;
    MoveCursor(Position, bInstant);
}

void UNsUICursorWidget::HideCursor()
{
    bShowing = false;
    TargetWidget.Reset();
    TargetMode.Reset();
    SetVisibility(ESlateVisibility::Collapsed);
}

bool UNsUICursorWidget::OnCursorMoved_Implementation(FVector2D ViewportPosition, bool bInstant)
{
    return false;
}

void UNsUICursorWidget::MoveCursor(const FVector2D& ViewportPosition, const bool bInstant)
{
    if (!OnCursorMoved(ViewportPosition, bInstant))
    {
        SetPositionInViewport(ViewportPosition, false);
    }
}

bool UNsUICursorWidget::GetTargetPosition(FVector2D& OutViewportPosition)
{
    if (const UNsUISelectionMode* const Mode = TargetMode.Get())
    {
        return Mode->GetSelectionPosition(OutViewportPosition);
    }
    const TSharedPtr<SWidget> Target = TargetWidget.Pin();
    if (!Target.IsValid())
    {
        return false;
    }
    const FGeometry& Geometry = Target->GetTickSpaceGeometry();
    if (Geometry.GetLocalSize().IsNearlyZero())
    {
        return false;
    }
    FVector2D Pixels;
    USlateBlueprintLibrary::LocalToViewport(this, Geometry, Geometry.GetLocalSize() * AnchorInTarget, Pixels, OutViewportPosition);
    return true;
}
