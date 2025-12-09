#include "NsUILayout.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "NsUICursor.h"
#include "NsUILayer.h"
#include "Styling/AppStyle.h"

class UCanvasPanel;

UNsUILayout::UNsUILayout(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , CursorAnchor(FVector2D(1.0f, 0.5f))
    , CursorWidgetSnapAnchor(FVector2D(0.0f, 0.5f))
{
    LayerClass = UNsUILayer::StaticClass();
    CursorBrush = *FAppStyle::Get().GetBrush("Icons.ChevronRight");
}

TSharedRef<SWidget> UNsUILayout::RebuildWidget()
{
    if (WidgetTree == nullptr)
    {
        WidgetTree = NewObject<UWidgetTree>(this, UWidgetTree::StaticClass(), TEXT("WidgetTree"));
    }

    UWidget* ExistingRoot = WidgetTree->RootWidget;

    UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(ExistingRoot);
    if (!RootCanvas)
    {
        RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
    }

    RootOverlay = Cast<UOverlay>(ExistingRoot);
    if (!RootOverlay)
    {
        RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
    }

    WidgetTree->RootWidget = RootCanvas;

    // Preserve any designer-built root by nesting it inside our overlay so it stretches with the
    // layout instead of being discarded when we rebuild programmatically.
    if (ExistingRoot && ExistingRoot != RootCanvas && ExistingRoot != RootOverlay && RootOverlay->GetChildrenCount() == 0)
    {
        if (UOverlaySlot* const ExistingSlot = RootOverlay->AddChildToOverlay(ExistingRoot))
        {
            ExistingSlot->SetHorizontalAlignment(HAlign_Fill);
            ExistingSlot->SetVerticalAlignment(VAlign_Fill);
        }
    }

    if (RootOverlay->Slot == nullptr || RootOverlay->Slot->Parent != RootCanvas)
    {
        if (UCanvasPanelSlot* OverlaySlot = RootCanvas->AddChildToCanvas(RootOverlay))
        {
            OverlaySlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
            OverlaySlot->SetAlignment(FVector2D::ZeroVector);
            OverlaySlot->SetOffsets(FMargin(0.f));
            OverlaySlot->SetZOrder(0);
        }
    }

    BuildLayer(ENsUILayer::Game, 0);
    BuildLayer(ENsUILayer::Menu, 1);
    BuildLayer(ENsUILayer::Modal, 2);

    CursorWidget = WidgetTree->ConstructWidget<UNsUICursor>(UNsUICursor::StaticClass());
    if (CursorWidget)
    {
        CursorWidget->SetCursorBrush(CursorBrush);

        // This puts it above Game/Menu/Modal because it’s added last
        RootOverlay->AddChildToOverlay(CursorWidget);
    }

    return Super::RebuildWidget();
}

UNsUILayer* UNsUILayout::GetLayerWidget(ENsUILayer Layer) const
{
    if (const TObjectPtr<UNsUILayer>* Found = Layers.Find(Layer))
    {
        return Found->Get();
    }

    return nullptr;
}

void UNsUILayout::UpdateCursorPosition(const FVector2D& ScreenPosition)
{
    if (CursorWidget)
    {
        const FVector2D Size = CursorWidget->GetDesiredSize();
        const FVector2D Offset = Size * CursorAnchor;

        CursorWidget->SetRenderTranslation(ScreenPosition - Offset);
    }
}

void UNsUILayout::SetCursorAnchor(const FVector2D& InAnchor)
{
    CursorAnchor.X = FMath::Clamp(InAnchor.X, 0.f, 1.f);
    CursorAnchor.Y = FMath::Clamp(InAnchor.Y, 0.f, 1.f);
}

void UNsUILayout::SetCursorVisibility(bool bVisible)
{
    if (CursorWidget)
    {
        CursorWidget->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Visible); // ESlateVisibility::Collapsed
    }
}

void UNsUILayout::BuildLayer(ENsUILayer Layer, int32 ZOrder)
{
    if (!LayerClass)
    {
        return;
    }

    UNsUILayer* LayerWidget = WidgetTree->ConstructWidget<UNsUILayer>(LayerClass);
    if (LayerWidget && RootOverlay)
    {
        LayerWidget->SetLayerType(Layer);
        if (UOverlaySlot* const NewLayerSlot = RootOverlay->AddChildToOverlay(LayerWidget)) /**->SetZOrder(ZOrder)*/
        {
            NewLayerSlot->SetHorizontalAlignment(HAlign_Fill);
            NewLayerSlot->SetVerticalAlignment(VAlign_Fill);
            Layers.Add(Layer, LayerWidget);
        }
    }
}
