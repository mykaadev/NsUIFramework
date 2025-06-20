#include "NsUICursor.h"
#include "Widgets/SViewport.h"

void UNsUICursor::MoveToWidget(UWidget* InTarget)
{
    //----------------------------------------------------------------------
    // 0) Guard clauses
    //----------------------------------------------------------------------
    if (!IsValid(InTarget))
    {
        SetVisibility(ESlateVisibility::Hidden);
        return;
    }

    //----------------------------------------------------------------------
    // 1) Make sure the target’s geometry exists
    //    - Force a one-frame layout pass if needed
    //----------------------------------------------------------------------
    InTarget->ForceLayoutPrepass();          // builds the Slate widget if this is the very first time

    const FGeometry TargetGeo = InTarget->GetCachedGeometry();
    if (!InTarget->GetCachedWidget().IsValid())
    {
        // Still no geometry?  Bail early – we’ll catch it next frame.
        return;
    }

    //----------------------------------------------------------------------
    // 2) Get the target’s top-left corner in absolute (desktop) space
    //----------------------------------------------------------------------
    const FVector2D TargetTopLeft = TargetGeo.LocalToAbsolute(FVector2D::ZeroVector);
    const FVector2D TargetSize    = TargetGeo.GetLocalSize();

    //----------------------------------------------------------------------
    // 3) Convert absolute → viewport space
    //    Slate gives us the geometry of the **game viewport widget**,
    //    so we can translate absolute pixels into viewport-local pixels.
    //----------------------------------------------------------------------
    FVector2D ViewportPos = TargetTopLeft;
    if (GEngine && GEngine->GameViewport)
    {
        const TSharedPtr<SViewport> ViewportWidget = GEngine->GameViewport->GetGameViewportWidget();
        const FGeometry ViewportGeo                = ViewportWidget->GetTickSpaceGeometry();

        // Absolute → viewport-local
        ViewportPos = ViewportGeo.AbsoluteToLocal(TargetTopLeft);
    }

    //----------------------------------------------------------------------
    // 4) Decide the cursor’s own size (skip GetDesiredSize – too early)
    //----------------------------------------------------------------------
    const FVector2D CursorSize(64.0f, 64.0f);   // pick whatever you need

    //----------------------------------------------------------------------
    // 5) Final position: left-align, vertically centered on the target
    //----------------------------------------------------------------------
    const FVector2D DesiredPos(
        ViewportPos.X,
        ViewportPos.Y + (TargetSize.Y - CursorSize.Y) * 0.5f);

    //----------------------------------------------------------------------
    // 6) Apply & show
    //----------------------------------------------------------------------
    SetPositionInViewport(DesiredPos, /*bRemoveDPIScale=*/false);
    SetDesiredSizeInViewport(CursorSize);
    SetVisibility(ESlateVisibility::HitTestInvisible);
}
