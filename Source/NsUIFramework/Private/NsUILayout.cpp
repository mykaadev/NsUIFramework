#include "NsUILayout.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "NsUICursor.h"
#include "NsUILayer.h"
#include "Styling/AppStyle.h"

UNsUILayout::UNsUILayout(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
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

    RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
    WidgetTree->RootWidget = RootOverlay;

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
        CursorWidget->SetRenderTranslation(ScreenPosition);
    }
}

void UNsUILayout::SetCursorVisibility(bool bVisible)
{
    if (CursorWidget)
    {
        CursorWidget->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
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
