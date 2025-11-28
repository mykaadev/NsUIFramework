// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUILayer.h"
#include "INsUINavigation.h"
#include "NsUISubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"

void UNsUILayer::NativeConstruct()
{
    Super::NativeConstruct();

    if (LayerRoot == nullptr)
    {
        LayerRoot = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RootBorder"));
        LayerRoot->SetPadding(FMargin(0.f));
        LayerRoot->SetBrushColor(FLinearColor::Transparent);
        LayerRoot->SetHorizontalAlignment(HAlign_Fill);
        LayerRoot->SetVerticalAlignment(VAlign_Fill);

        WidgetTree->RootWidget = LayerRoot;

        const auto Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DebugText"));
        Text->SetText(FText::FromString(TEXT("Hello, world!")));
        LayerRoot->SetContent(Text);
    }
}

UNsUIWidgetStackElement* UNsUILayer::Push(const TSoftClassPtr<UUserWidget>& InClass)
{
    UNsUISubsystem* UISubsystem = UNsUISubsystem::GetSubsystem();
    if (LayerRoot != nullptr && InClass != nullptr && UISubsystem != nullptr)
    {
        if (APlayerController* const OwningPlayer = GetOwningPlayer())
        {
            if (UUserWidget* const NewWidget = CreateWidget(OwningPlayer, InClass.LoadSynchronous()))
            {
                // run push logic from the interface
                PushWidget(NewWidget);

                // create new stack element
                UNsUIWidgetStackElement* NewStackElement = NewObject<UNsUIWidgetStackElement>(OwningPlayer);
                NewStackElement->WidgetScope = NewWidget;
                NewStackElement->WidgetScopeFocusedElement = GetInitialFocusForWidget(NewWidget);
                NewStackElement->InvokerWidgetScope = UISubsystem->GetCurrentScope() != nullptr ? UISubsystem->GetCurrentScope()->WidgetScope : nullptr;
                NewStackElement->InvokerElement = UISubsystem->GetCurrentScope() != nullptr ? UISubsystem->GetCurrentScope()->WidgetScopeFocusedElement : nullptr;
                NewStackElement->InvokerLayerType = UISubsystem->GetCurrentScope() != nullptr ? UISubsystem->GetCurrentScope()->WidgetScopeLayerType : ENsUILayerType::None;
                
                // handle the push to the layer
                CollapseTop();                      // hide the current top element
                LayerRoot->ClearChildren();         // clear the root
                Stack.Emplace(NewStackElement);     // add new element to stack
                LayerRoot->AddChild(NewWidget);     // push it to the root
                ShowTop();                          // show the new element on top of the stack
                return Peek();                      // Peek() now gives us this new element
            }
        }
    }
    
    return nullptr;
}

UNsUIWidgetStackElement* UNsUILayer::Pop()
{
    if (!Stack.IsEmpty())
    {
        // Pop the top widget out of the stack
        UNsUIWidgetStackElement* TopElement = Peek();
        if (TopElement != nullptr && TopElement->WidgetScope != nullptr && LayerRoot != nullptr)
        {
            UNsUIWidgetStackElement* CachedInfoInvoker = TopElement;

            // Remove the current widget
            TopElement->WidgetScope->RemoveFromParent();
            Stack.Remove(TopElement);
            LayerRoot->ClearChildren();

            // Enable the one that is now on the top of the stack
            TopElement = Peek();
            if (TopElement != nullptr && TopElement->WidgetScope != nullptr)
            {
                TopElement->WidgetScope->SetVisibility(ESlateVisibility::Visible);
                LayerRoot->AddChild(TopElement->WidgetScope);
                return TopElement;
            }
            else // we don't have any more widgets in this layer so we must pass the invoker info to the layout so he can handle it
            {
                return CachedInfoInvoker;
            }
        }
    }

    return nullptr;
}

UNsUIWidgetStackElement* UNsUILayer::RemoveWidgetFromStack(UUserWidget* InWidget)
{
    if (!InWidget)
    {
        return Peek();
    }

    const int32 FoundIndex = Stack.IndexOfByPredicate([InWidget](const UNsUIWidgetStackElement* Element)
    {
        return Element != nullptr && Element->WidgetScope == InWidget;
    });

    if (FoundIndex != INDEX_NONE)
    {
        UNsUIWidgetStackElement* RemovedElement = Stack[FoundIndex];

        if (RemovedElement != nullptr && RemovedElement->WidgetScope != nullptr)
        {
            RemovedElement->WidgetScope->RemoveFromParent();
        }

        Stack.RemoveAt(FoundIndex);

        if (LayerRoot != nullptr)
        {
            LayerRoot->ClearChildren();
        }

        if (UNsUIWidgetStackElement* const NewTop = Peek())
        {
            if (NewTop->WidgetScope != nullptr)
            {
                NewTop->WidgetScope->SetVisibility(ESlateVisibility::Visible);

                if (LayerRoot != nullptr)
                {
                    LayerRoot->AddChild(NewTop->WidgetScope);
                }

                return NewTop;
            }
        }

        return RemovedElement;
    }

    return Peek();
}

TArray<UNsUIWidgetStackElement*> UNsUILayer::GetStack() const
{
    return Stack;
}

void UNsUILayer::ClearStack()
{
    for (UNsUIWidgetStackElement* StackElement : Stack)
    {
        if (StackElement->WidgetScope != nullptr)
        {
            StackElement->WidgetScope->RemoveFromParent();
        }
    }

    Stack.Empty();

    if (LayerRoot != nullptr)
    {
        LayerRoot->ClearChildren();
    }
}

UNsUIWidgetStackElement* UNsUILayer::Peek()
{
    return !Stack.IsEmpty() ? Stack.Last() : nullptr;
}

void UNsUILayer::CollapseTop()
{
    if (UNsUIWidgetStackElement* TopElement = Peek())
    {
        if (TopElement->WidgetScope != nullptr)
        {
            TopElement->WidgetScope->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void UNsUILayer::ShowTop()
{
    if (UNsUIWidgetStackElement* const TopElement = Peek())
    {
        if (TopElement->WidgetScope != nullptr)
        {
            TopElement->WidgetScope->SetVisibility(ESlateVisibility::Visible);
        }
    }
}

UNsUIWidgetStackElement* UNsUILayer::FindStackElement(const UUserWidget* InInvokerScope, const UWidget* InInvokerElement)
{
    for (int32 Index = Stack.Num() - 1; Index >= 0; --Index)
    {
        if (Stack[Index] != nullptr)
        {
            if (Stack[Index]->WidgetScope == InInvokerScope && Stack[Index]->WidgetScopeFocusedElement == InInvokerElement)
            {
                return Stack[Index];
            }
        }
    }

    return nullptr;
}

FText UNsUILayer::GetStackListNames()
{
    TArray<FString> StackListNames;

    for (int32 Index = Stack.Num() - 1; Index >= 0; --Index)
    {
        if (Stack[Index]->WidgetScope != nullptr)
        {
            StackListNames.Emplace(Stack[Index]->WidgetScope->GetName());
        }
    }

    return FText::FromString(FString::Join(StackListNames, TEXT("\n")));
}

UWidget* UNsUILayer::GetInitialFocusForWidget(UUserWidget* InWidget)
{
    if (InWidget != nullptr &&
        InWidget->GetClass() != nullptr &&
        InWidget->GetClass()->ImplementsInterface(UNsUINavigation::StaticClass()))
    {
        if (const INsUINavigation* const NavigationInterface = Cast<INsUINavigation>(InWidget))
        {
            return NavigationInterface->Execute_GetInitialFocusWidget(InWidget);
        }
    }

    return nullptr;
}

void UNsUILayer::PushWidget(UUserWidget* InWidget)
{
    if (InWidget != nullptr &&
        InWidget->GetClass() != nullptr &&
        InWidget->GetClass()->ImplementsInterface(UNsUINavigation::StaticClass()))
    {
        if (const INsUINavigation* const NavigationInterface = Cast<INsUINavigation>(InWidget))
        {
            NavigationInterface->Execute_OnWidgetPushed(InWidget);
        }
    }
}