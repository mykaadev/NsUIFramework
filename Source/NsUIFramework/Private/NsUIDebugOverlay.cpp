#include "NsUIDebugOverlay.h"

#if WITH_EDITOR

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

UNsUIDebugOverlay::UNsUIDebugOverlay(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , RootBox(nullptr)
{
}

TSharedRef<SWidget> UNsUIDebugOverlay::RebuildWidget()
{
    if (WidgetTree)
    {
        UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
        RootBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

        if (RootOverlay && RootBox)
        {
            UOverlaySlot* OverlaySlot = RootOverlay->AddChildToOverlay(RootBox);
            OverlaySlot->SetHorizontalAlignment(HAlign_Center);
            OverlaySlot->SetVerticalAlignment(VAlign_Center);
            WidgetTree->RootWidget = RootOverlay;
        }
    }

    return Super::RebuildWidget();
}

void UNsUIDebugOverlay::SetSnapshots(const TArray<FNsUILayerSnapshot>& Snapshots)
{
    if (!RootBox)
    {
        return;
    }

    RootBox->ClearChildren();
    BuildEntries(Snapshots);
}

void UNsUIDebugOverlay::BuildEntries(const TArray<FNsUILayerSnapshot>& Snapshots)
{
    for (const FNsUILayerSnapshot& Snapshot : Snapshots)
    {
        const FString Title = Snapshot.Layer == ENsUILayer::Game ? TEXT("Game Layer") :
            (Snapshot.Layer == ENsUILayer::Menu ? TEXT("Menu Layer") : TEXT("Modal Layer"));

        if (UVerticalBox* Column = AddLayerColumn(Title))
        {
            for (int32 Index = 0; Index < Snapshot.WidgetNames.Num(); ++Index)
            {
                const bool bFocused = Snapshot.FocusedIndex == Index;
                const FString Prefix = bFocused ? TEXT("> ") : TEXT("- ");
                const FString EntryText = Prefix + Snapshot.WidgetNames[Index];

                UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                Text->SetText(FText::FromString(EntryText));
                Column->AddChildToVerticalBox(Text);
            }
        }
    }
}

UVerticalBox* UNsUIDebugOverlay::AddLayerColumn(const FString& Title)
{
    if (!RootBox)
    {
        return nullptr;
    }

    UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
    UTextBlock* Header = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    Header->SetText(FText::FromString(Title));

    Column->AddChildToVerticalBox(Header);

    RootBox->AddChildToHorizontalBox(Column);

    return Column;
}

#endif
