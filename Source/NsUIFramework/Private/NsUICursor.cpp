// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUICursor.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
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
    // Hide if no valid target.
    if (!IsValid(InTarget) || !InTarget->IsVisible())
    {
        if (bHideWhenNoTarget)
        {
            SetVisibility(ESlateVisibility::Collapsed);
        }
        return;
    }

    // Ensure Slate widgets exist.
    const TSharedPtr<SWidget> TargetSlate = InTarget->GetCachedWidget();
    if (!TargetSlate.IsValid())
    {
        if (UWorld* const World = GetWorld())
        {
            World->GetTimerManager().SetTimerForNextTick(
                FTimerDelegate::CreateUObject(this, &UNsUICursor::MoveToWidget, InTarget));
        }
        return;
    }

    // Make sure geometry is fresh on both sides.
    if (UUserWidget* const TargetRoot = InTarget->GetTypedOuter<UUserWidget>())
    {
        TargetRoot->ForceLayoutPrepass();
    }
    if (UUserWidget* const CursorRoot = GetTypedOuter<UUserWidget>())
    {
        CursorRoot->ForceLayoutPrepass();
    }

    // We’ll compute everything from absolute space.
    const FGeometry ViewportGeo = UWidgetLayoutLibrary::GetViewportWidgetGeometry(this);
    if (ViewportGeo.GetLocalSize().IsNearlyZero())
    {
        if (UWorld* const World = GetWorld())
        {
            World->GetTimerManager().SetTimerForNextTick(
                FTimerDelegate::CreateUObject(this, &UNsUICursor::MoveToWidget, InTarget));
        }
        return;
    }

    const FGeometry TargetGeo = InTarget->GetCachedGeometry();
    const FVector2D TargetSize = TargetGeo.GetLocalSize();
    if (TargetSize.IsNearlyZero())
    {
        if (UWorld* const World = GetWorld())
        {
            World->GetTimerManager().SetTimerForNextTick(
                FTimerDelegate::CreateUObject(this, &UNsUICursor::MoveToWidget, InTarget));
        }
        return;
    }

    // Anchor in absolute (desktop) space.
    const FVector2D AbsAnchor = TargetGeo.LocalToAbsolute(TargetAnchor * TargetSize);

    // Get the cursor’s pixel size.
    FVector2D CursorSize = GetDesiredSize();
    if (CursorSize.IsNearlyZero())
    {
        SynchronizeProperties();
        InvalidateLayoutAndVolatility();
        CursorSize = GetDesiredSize();
    }

    // If the cursor is a top-level viewport widget, position in viewport space.
    if (IsInViewport())
    {
        const FVector2D ViewportLocal = ViewportGeo.AbsoluteToLocal(AbsAnchor);
        const FVector2D DesiredPosViewport =
            ViewportLocal - (CursorSize * CursorAlignment) + FVector2D(HorizontalPadding, 0.0f);

        SetPositionInViewport(DesiredPosViewport, /*bRemoveDPIScale=*/false);
        SetDesiredSizeInViewport(CursorSize);
        SetRenderTransformPivot(CursorAlignment);
#if !UE_BUILD_SHIPPING
        UE_LOG(LogTemp, Warning, TEXT("[Cursor] VP LocalPosition: X:%.6f Y:%.6f"), DesiredPosViewport.X, DesiredPosViewport.Y);
#endif
    }
    else
    {
        // Cursor is inside your Layout tree. Place it in the parent’s local space.
        UPanelWidget* const Parent = GetParent();
        if (!Parent)
        {
            // No parent yet — try again next tick.
            if (UWorld* const World = GetWorld())
            {
                World->GetTimerManager().SetTimerForNextTick(
                    FTimerDelegate::CreateUObject(this, &UNsUICursor::MoveToWidget, InTarget));
            }
            return;
        }

        // Convert absolute anchor to the parent’s local space.
        const FGeometry ParentGeo = Parent->GetCachedGeometry();
        const FVector2D ParentLocal = ParentGeo.AbsoluteToLocal(AbsAnchor);
        const FVector2D DesiredLocal =
            ParentLocal - (CursorSize * CursorAlignment) + FVector2D(HorizontalPadding, 0.0f);

        // Best: if parent is a Canvas, set slot position (authoritative, no extra transforms).
        if (UCanvasPanelSlot* const CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
        {
            CanvasSlot->SetAlignment(CursorAlignment);
            CanvasSlot->SetAutoSize(true);
            CanvasSlot->SetPosition(DesiredLocal);
            CanvasSlot->SetZOrder(9999); // ensure on top within the Layout
#if !UE_BUILD_SHIPPING
            UE_LOG(LogTemp, Warning, TEXT("[Cursor] Canvas LocalPosition: X:%.6f Y:%.6f"), DesiredLocal.X, DesiredLocal.Y);
#endif
        }
        else
        {
            // Fallback for Overlay/VerticalBox/etc.: use render translation in parent space.
            SetRenderTransformPivot(CursorAlignment);
            FWidgetTransform Xform = RenderTransform;
            Xform.Translation = DesiredLocal;
            SetRenderTransform(Xform);
#if !UE_BUILD_SHIPPING
            UE_LOG(LogTemp, Warning, TEXT("[Cursor] RenderXform LocalPosition: X:%.2f Y:%.2f"), DesiredLocal.X, DesiredLocal.Y);
#endif
        }
    }

    SetVisibility(ESlateVisibility::SelfHitTestInvisible);
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

    SetVisibility(ESlateVisibility::Visible);
}
