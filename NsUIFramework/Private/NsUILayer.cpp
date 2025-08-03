// Copyright (C) 2024 mykaa. All rights reserved.

#include "NsUILayer.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"

void UNsUILayer::NativeConstruct()
{
    Super::NativeConstruct();

    if (!RootBorder)
    {
        RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RootBorder"));
        RootBorder->SetPadding(FMargin(0.f));
        RootBorder->SetBrushColor(FLinearColor::Transparent);
        RootBorder->SetHorizontalAlignment(HAlign_Fill);
        RootBorder->SetVerticalAlignment(VAlign_Fill);

        WidgetTree->RootWidget = RootBorder;

        const auto Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DebugText"));
        Text->SetText(FText::FromString(TEXT("Hello, world!")));
        RootBorder->SetContent(Text);
    }
}
UUserWidget* UNsUILayer::Push(const TSoftClassPtr<UUserWidget>& InClass)
{
    if (InClass != nullptr)
    {
        if (APlayerController* const OwningPlayer = GetOwningPlayer())
        {
            if (UUserWidget* const PushedWidget = CreateWidget(OwningPlayer, InClass.LoadSynchronous()))
            {
                if (RootBorder != nullptr)
                {
                    CollapseTop();

                    RootBorder->ClearChildren();
                    WidgetStack.Emplace(PushedWidget);
                    RootBorder->AddChild(PushedWidget);

                    ShowTop();

                    return PushedWidget;
                }
            }
        }
    }
    
    return nullptr;
}

UUserWidget* UNsUILayer::Pop()
{
    if (!WidgetStack.IsEmpty())
    {
        UUserWidget* LastWidget = Peek();
        if (LastWidget != nullptr && RootBorder != nullptr)
        {
            LastWidget->RemoveFromParent();
            WidgetStack.Remove(LastWidget);
            RootBorder->ClearChildren();

            LastWidget = Peek(); // Search for the top widget in the stack again
            if (LastWidget != nullptr)
            {
                LastWidget->SetVisibility(ESlateVisibility::Visible);
                RootBorder->AddChild(LastWidget);
                return LastWidget;
            }
        }
    }

    return nullptr;
}

void UNsUILayer::ClearStack()
{
    for (UUserWidget* const Widget : WidgetStack)
    {
        if (Widget != nullptr)
        {
            Widget->RemoveFromParent();
        }
    }

    WidgetStack.Empty();

    if (RootBorder != nullptr)
    {
        RootBorder->ClearChildren();
    }
}

class UUserWidget* UNsUILayer::Peek()
{
    UUserWidget* Output = nullptr;
    if (!WidgetStack.IsEmpty())
    {
        Output = WidgetStack.Last();
    }
    return Output;
}

void UNsUILayer::CollapseTop()
{
    if (UUserWidget* const LastWidget = Peek();
        LastWidget != nullptr)
    {
        LastWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UNsUILayer::ShowTop()
{
    if (UUserWidget* const LastWidget = Peek();
        LastWidget != nullptr)
    {
        LastWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
}

FText UNsUILayer::GetStackListNames()
{
    TArray<FString> StackListNames;
    for (int32 i = WidgetStack.Num() - 1; i >= 0; --i)
    {
        if (WidgetStack[i] != nullptr)
        {
            StackListNames.Emplace(WidgetStack[i]->GetName());
        }
    }

    return FText::FromString(FString::Join(StackListNames, TEXT("\n")));
}
