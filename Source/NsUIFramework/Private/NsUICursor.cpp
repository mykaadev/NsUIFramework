#include "NsUICursor.h"
#include "Widgets/SViewport.h"

UNsUICursor::UNsUICursor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , TargetAnchor(FVector2D(0.0f, 0.25f))
    , CursorAlignment(FVector2D(0.0f, 0.0f))
{
}

void UNsUICursor::MoveToWidget(UWidget* const InTarget)
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
    InTarget->ForceLayoutPrepass(); // builds the Slate widget if this is the very first time

    const FGeometry TargetGeo = InTarget->GetCachedGeometry();
    if (!InTarget->GetCachedWidget().IsValid())
    {
        // Still no geometry?  Bail early – we’ll catch it next frame.
        if (UWorld* const World = GetWorld())
        {
            World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UNsUICursor::MoveToWidget, InTarget));
        }

        return;
    }

    //----------------------------------------------------------------------
    // 2) Get the target’s anchor point in absolute (desktop) space
    //----------------------------------------------------------------------
    const FVector2D TargetTopLeft = TargetGeo.LocalToAbsolute(FVector2D::ZeroVector);
    const FVector2D TargetSize    = TargetGeo.GetLocalSize();
    const FVector2D TargetAnchorPos = TargetTopLeft + TargetSize * TargetAnchor;

    //----------------------------------------------------------------------
    // 3) Convert absolute → viewport space
    //    Slate gives us the geometry of the **game viewport widget**,
    //    so we can translate absolute pixels into viewport-local pixels.
    //----------------------------------------------------------------------
    FVector2D ViewportPos = TargetAnchorPos;
    if (GEngine != nullptr && GEngine->GameViewport)
    {
        const TSharedPtr<SViewport> ViewportWidget = GEngine->GameViewport->GetGameViewportWidget();
        const FGeometry ViewportGeo                = ViewportWidget->GetTickSpaceGeometry();

        // Absolute → viewport-local
        ViewportPos = ViewportGeo.AbsoluteToLocal(TargetAnchorPos);
    }

    //----------------------------------------------------------------------
    // 4) Decide the cursor’s own size (skip GetDesiredSize – too early)
    //----------------------------------------------------------------------
    ForceLayoutPrepass();  // build this widget’s geometry
    const FVector2D CursorSize = GetDesiredSize();

    //----------------------------------------------------------------------
    // 5) Final position: left-align, vertically centered on the target
    //----------------------------------------------------------------------
    const FVector2D DesiredPos = ViewportPos - CursorSize * CursorAlignment;

    //----------------------------------------------------------------------
    // 6) Apply & show
    //----------------------------------------------------------------------
    SetAlignmentInViewport(CursorAlignment);
    SetPositionInViewport(DesiredPos, /*bRemoveDPIScale=*/true);
    SetDesiredSizeInViewport(CursorSize);
    SetVisibility(ESlateVisibility::HitTestInvisible);
}
