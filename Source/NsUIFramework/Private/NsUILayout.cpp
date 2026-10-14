#include "NsUILayout.h"
#include "NsUILayer.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Styling/AppStyle.h"

UNsUILayout::UNsUILayout(const FObjectInitializer& Initializer) : Super(Initializer)
{
    CursorBrush = *FAppStyle::Get().GetBrush("Icons.ChevronRight");
    SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

TSharedRef<SWidget> UNsUILayout::RebuildWidget()
{
    if (!WidgetTree->RootWidget)
    {
        UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>();
        Root->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        WidgetTree->RootWidget = Root;
        for (const ENsUILayer Layer : { ENsUILayer::Game, ENsUILayer::Menu, ENsUILayer::Modal })
        {
            UNsUILayer* Panel = WidgetTree->ConstructWidget<UNsUILayer>();
            Panel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
            UOverlaySlot* LayerSlot = Root->AddChildToOverlay(Panel);
            LayerSlot->SetHorizontalAlignment(HAlign_Fill);
            LayerSlot->SetVerticalAlignment(VAlign_Fill);
            Layers.Add(Layer, Panel);
        }
    }
    return Super::RebuildWidget();
}

UNsUILayer* UNsUILayout::GetLayerWidget(ENsUILayer Layer) const
{
    return Layers.FindRef(Layer);
}
