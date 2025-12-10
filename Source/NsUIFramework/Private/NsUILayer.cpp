#include "NsUILayer.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/PanelWidget.h"

UNsUILayer::UNsUILayer(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , RootPanel(nullptr)
    , Layer(ENsUILayer::Game)
{
    SetIsFocusable(false);
}

TSharedRef<SWidget> UNsUILayer::RebuildWidget()
{
    if (WidgetTree == nullptr)
    {
        WidgetTree = NewObject<UWidgetTree>(this, UWidgetTree::StaticClass(), TEXT("WidgetTree"));
    }

    if (WidgetTree->RootWidget == nullptr)
    {
        RootPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LayerRoot"));
        WidgetTree->RootWidget = RootPanel;

        RootPanel->SetPadding(FMargin(0.f));
        RootPanel->SetHorizontalAlignment(HAlign_Fill);
        RootPanel->SetVerticalAlignment(VAlign_Fill);

        // Kill the white background
        FSlateBrush Brush;
        //Brush.DrawAs = ESlateBrushDrawType::NoDrawType;
        Brush.TintColor = FSlateColor(FLinearColor::Transparent); // FLinearColor::Transparent
        RootPanel->SetBrush(Brush);

        RootPanel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
    else
    {
        RootPanel = Cast<UBorder>(WidgetTree->RootWidget);
    }

    return Super::RebuildWidget();
}

void UNsUILayer::AddScreen(UUserWidget* Screen)
{
    if (RootPanel && Screen)
    {
        RootPanel->AddChild(Screen);
    }
}

void UNsUILayer::RemoveScreen(UUserWidget* Screen)
{
    if (RootPanel && Screen)
    {
        RootPanel->RemoveChild(Screen);
    }
}

void UNsUILayer::ClearScreens()
{
    if (RootPanel)
    {
        RootPanel->ClearChildren();
    }
}

void UNsUILayer::SetLayerType(ENsUILayer InLayer)
{
    Layer = InLayer;
}
