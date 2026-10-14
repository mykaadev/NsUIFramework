#include "NsUILayer.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/PanelWidget.h"

UNsUILayer::UNsUILayer(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , RootOverlay(nullptr)
{
    SetIsFocusable(false);
}

TSharedRef<SWidget> UNsUILayer::RebuildWidget()
{
    if (WidgetTree == nullptr)
    {
        WidgetTree = NewObject<UWidgetTree>(this, UWidgetTree::StaticClass(), TEXT("WidgetTree"));
    }

    RootOverlay = Cast<UOverlay>(WidgetTree->RootWidget);
    if (RootOverlay == nullptr)
    {
        RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("LayerRoot"));
        WidgetTree->RootWidget = RootOverlay;
    }

    RootOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

    return Super::RebuildWidget();
}

void UNsUILayer::AddScreen(UUserWidget* Screen)
{
    if (RootOverlay && Screen)
    {
        UOverlaySlot* OverlaySlot = RootOverlay->AddChildToOverlay(Screen);
        if (OverlaySlot)
        {
            OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
            OverlaySlot->SetVerticalAlignment(VAlign_Fill);
        }
    }
}
