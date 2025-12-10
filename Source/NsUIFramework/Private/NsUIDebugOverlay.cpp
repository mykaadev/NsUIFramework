#include "NsUIDebugOverlay.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
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
    , BackgroundBorder(nullptr)
{
    // This widget updates every frame in some cases; volatility avoids extra invalidation churn
    bIsVolatile = true;

    // Don’t block input – we’ll enforce this again after RebuildWidget as well
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

TSharedRef<SWidget> UNsUIDebugOverlay::RebuildWidget()
{
    if (WidgetTree)
    {
        UOverlay* const RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
        BackgroundBorder            = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
        RootBox                     = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

        if (RootOverlay && BackgroundBorder && RootBox)
        {
            // Style: small, top-left, semi-transparent black
            BackgroundBorder->SetPadding(FMargin(8.f, 4.f));
            BackgroundBorder->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.65f));
            BackgroundBorder->SetHorizontalAlignment(HAlign_Left);
            BackgroundBorder->SetVerticalAlignment(VAlign_Top);
            BackgroundBorder->SetContent(RootBox);

            UOverlaySlot* OverlaySlot = RootOverlay->AddChildToOverlay(BackgroundBorder);
            OverlaySlot->SetHorizontalAlignment(HAlign_Left);
            OverlaySlot->SetVerticalAlignment(VAlign_Top);
            OverlaySlot->SetPadding(FMargin(12.f, 8.f)); // offset from top-left corner a bit

            WidgetTree->RootWidget = RootOverlay;
        }
    }

    // Build the underlying slate widget
    TSharedRef<SWidget> Result = Super::RebuildWidget();

    // Make sure overlay never captures input
    SetVisibility(ESlateVisibility::HitTestInvisible);

    return Result;
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
    // Small, condensed debug info
    for (const FNsUILayerSnapshot& Snapshot : Snapshots)
    {
        const FString Title = Snapshot.Layer == ENsUILayer::Game  ? TEXT("Game")
                            : Snapshot.Layer == ENsUILayer::Menu  ? TEXT("Menu")
                            : TEXT("Modal");

        if (UVerticalBox* const Column = AddLayerColumn(Title))
        {
            for (int32 Index = 0; Index < Snapshot.WidgetNames.Num(); ++Index)
            {
                const bool bFocused = Snapshot.FocusedIndex == Index;
                const FString Prefix   = bFocused ? TEXT("> ") : TEXT("- ");
                const FString EntryStr = Prefix + Snapshot.WidgetNames[Index];

                UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                Text->SetText(FText::FromString(EntryStr));

                // Smaller, subtle font for entries
                FSlateFontInfo FontInfo = Text->GetFont();
                FontInfo.Size = 10;
                Text->SetFont(FontInfo);
                Text->SetColorAndOpacity(
                    bFocused
                        ? FSlateColor(FLinearColor(0.55f, 0.31f, 0.90f, 1.00f))   // highlight focused
                        : FSlateColor(FLinearColor(0.52f, 0.52f, 0.52f, 0.90f))
                );

                // Slight spacing between entries
                if (UVerticalBoxSlot* const VBSlot = Column->AddChildToVerticalBox(Text))
                {
                    VBSlot->SetPadding(FMargin(0.f, 1.f, 0.f, 0.f));
                }
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

    UVerticalBox* const Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
    UTextBlock* const Header = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());

    Header->SetText(FText::FromString(Title));

    // Slightly larger font for header
    FSlateFontInfo HeaderFont = Header->GetFont();
    HeaderFont.Size = 14;
    Header->SetFont(HeaderFont);
    Header->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f, 1.f)));

    if (UVerticalBoxSlot* const HeaderSlot = Column->AddChildToVerticalBox(Header))
    {
        HeaderSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 3.f));
    }

    if (UHorizontalBoxSlot* const ColumnSlot = RootBox->AddChildToHorizontalBox(Column))
    {
        ColumnSlot->SetPadding(FMargin(6.f, 0.f, 6.f, 0.f));    // spacing between columns
        ColumnSlot->SetHorizontalAlignment(HAlign_Left);
        ColumnSlot->SetVerticalAlignment(VAlign_Top);
    }

    return Column;
}