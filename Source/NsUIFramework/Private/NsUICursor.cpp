#include "NsUICursor.h"

#include "Components/PanelWidget.h"
#include "Widgets/SViewport.h"

UNsUICursor::UNsUICursor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , TargetAnchor(FVector2D(0.0f, 0.5f))
    , CursorAlignment(FVector2D(1.0f, 0.5f))
    , HorizontalPadding(6.0f)
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
    if (GEngine && GEngine->GameViewport)
    {
        const TSharedPtr<SViewport> ViewportWidget = GEngine->GameViewport->GetGameViewportWidget();
        if (ViewportWidget.IsValid())
        {
            const FGeometry ViewportGeo = ViewportWidget->GetCachedGeometry(); // safer than GetTickSpaceGeometry
            ViewportPos = ViewportGeo.AbsoluteToLocal(TargetAnchorPos);
        }
    }

    //----------------------------------------------------------------------
    // 4) Decide the cursor’s own size (skip GetDesiredSize – too early)
    //----------------------------------------------------------------------
    ForceLayoutPrepass();  // build this widget’s geometry
    const FVector2D CursorSize = GetDesiredSize();

    //----------------------------------------------------------------------
    // 5) Apply & show
    //    - If we're added directly to the viewport, use the viewport helpers
    //    - Otherwise, move via render translation so we work as a panel child
    //----------------------------------------------------------------------
    if (IsInViewport())
    {
        const FVector2D DesiredPos = ViewportPos - CursorSize * CursorAlignment + FVector2D(HorizontalPadding, 0.0f);
        SetAlignmentInViewport(CursorAlignment);
        SetPositionInViewport(DesiredPos, /*bRemoveDPIScale=*/false);
        SetDesiredSizeInViewport(CursorSize);
    }
    else
    {
        FVector2D LocalPos = FVector2D::ZeroVector;

        if (UPanelWidget* const ParentWidget = GetParent())
        {
            ParentWidget->ForceLayoutPrepass();
            const FGeometry ParentGeo = ParentWidget->GetCachedGeometry();

            const FVector2D TargetAnchorAbs = TargetGeo.LocalToAbsolute(TargetGeo.GetLocalSize() * TargetAnchor);
            const FVector2D LocalAnchor = ParentGeo.AbsoluteToLocal(TargetAnchorAbs);

            const FVector2D CursorOffset = CursorSize * CursorAlignment + FVector2D(HorizontalPadding, 0.0f);
            LocalPos = LocalAnchor - CursorOffset;
        }

        SetRenderTransformPivot(CursorAlignment);
        SetRenderTranslation(LocalPos);
    }

    SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UNsUICursor::MoveToScreenPosition(const FVector2D& InViewportPos)
{
    // Ensure our own geometry is up to date
    ForceLayoutPrepass();
    const FVector2D CursorSize = GetDesiredSize();

    // Convert viewport coordinates to absolute for non-viewport parents
    FVector2D AbsolutePos = InViewportPos;
    if (GEngine && GEngine->GameViewport)
    {
        const TSharedPtr<SViewport> ViewportWidget = GEngine->GameViewport->GetGameViewportWidget();
        if (ViewportWidget.IsValid())
        {
            const FGeometry ViewportGeo = ViewportWidget->GetCachedGeometry();
            AbsolutePos = ViewportGeo.LocalToAbsolute(InViewportPos);
        }
    }

    if (IsInViewport())
    {
        const FVector2D DesiredPos = InViewportPos - CursorSize * CursorAlignment + FVector2D(HorizontalPadding, 0.0f);
        SetAlignmentInViewport(CursorAlignment);
        SetPositionInViewport(DesiredPos, /*bRemoveDPIScale=*/false);
        SetDesiredSizeInViewport(CursorSize);
    }
    else if (UPanelWidget* const ParentWidget = GetParent())
    {
        ParentWidget->ForceLayoutPrepass();
        const FGeometry ParentGeo = ParentWidget->GetCachedGeometry();
        const FVector2D LocalPos = ParentGeo.AbsoluteToLocal(AbsolutePos) - CursorSize * CursorAlignment + FVector2D(HorizontalPadding, 0.0f);
        SetRenderTransformPivot(CursorAlignment);
        SetRenderTranslation(LocalPos);
    }

    SetVisibility(ESlateVisibility::HitTestInvisible);
}
