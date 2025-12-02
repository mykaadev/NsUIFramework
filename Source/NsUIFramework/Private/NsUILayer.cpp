// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUILayer.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/PanelWidget.h"
#include "NsUILibrary.h"
#include "NsUISubsystem.h"
#include "NsUIWidget.h"

void UNsUILayer::NativeConstruct()
{
    Super::NativeConstruct();
    Stack.Empty();
}

UNsUIWidgetStackElement* UNsUILayer::CreateStackElement(UUserWidget* Widget) const
{
    if (!Widget)
    {
        return nullptr;
    }

    UNsUIWidgetStackElement* Element = NewObject<UNsUIWidgetStackElement>(Widget);
    Element->Widget = Widget;
    Element->Layer = ENsUILayerType::None;
    return Element;
}

void UNsUILayer::HideCurrentTop()
{
    if (UNsUIWidgetStackElement* Top = Peek())
    {
        Top->Widget->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UNsUILayer::ShowCurrentTop()
{
    if (UNsUIWidgetStackElement* Top = Peek())
    {
        Top->Widget->SetVisibility(ESlateVisibility::Visible);
        Top->Widget->SetKeyboardFocus();
    }
}

UNsUIWidgetStackElement* UNsUILayer::Push(const TSoftClassPtr<UUserWidget>& InClass, bool bCollapsePrevious)
{
    APlayerController* OwningPlayer = GetOwningPlayer();
    if (!OwningPlayer || (!LayerRoot && !LayerPanel))
    {
        return nullptr;
    }

    UClass* WidgetClass = InClass.LoadSynchronous();
    if (!WidgetClass)
    {
        return nullptr;
    }

    UUserWidget* NewWidget = CreateWidget<UUserWidget>(OwningPlayer, WidgetClass);
    if (!NewWidget)
    {
        return nullptr;
    }

    if (bCollapsePrevious)
    {
        HideCurrentTop();
    }

    if (LayerPanel)
    {
        LayerPanel->AddChild(NewWidget);
    }
    else if (LayerRoot)
    {
        LayerRoot->SetContent(NewWidget);
    }

    UNsUIWidgetStackElement* Element = CreateStackElement(NewWidget);
    Stack.Add(Element);
    ShowCurrentTop();
    return Element;
}

UNsUIWidgetStackElement* UNsUILayer::ReplaceTop(const TSoftClassPtr<UUserWidget>& InClass)
{
    Pop();
    return Push(InClass);
}

UNsUIWidgetStackElement* UNsUILayer::Pop()
{
    if (Stack.Num() == 0)
    {
        return nullptr;
    }

    UNsUIWidgetStackElement* Removed = Stack.Pop();
    if (Removed && Removed->Widget)
    {
        if (LayerPanel)
        {
            LayerPanel->RemoveChild(Removed->Widget);
        }
        else if (LayerRoot && LayerRoot->GetContent() == Removed->Widget)
        {
            LayerRoot->SetContent(nullptr);
        }
        Removed->Widget->RemoveFromParent();
    }

    ShowCurrentTop();
    return Removed;
}

UNsUIWidgetStackElement* UNsUILayer::RemoveWidgetFromStack(UUserWidget* InWidget)
{
    if (!InWidget)
    {
        return nullptr;
    }

    const int32 Index = Stack.IndexOfByPredicate([InWidget](const UNsUIWidgetStackElement* Element)
    {
        return Element && Element->Widget == InWidget;
    });

    if (Index == INDEX_NONE)
    {
        return nullptr;
    }

    UNsUIWidgetStackElement* Removed = Stack[Index];
    Stack.RemoveAt(Index);

    if (Removed && Removed->Widget)
    {
        if (LayerPanel)
        {
            LayerPanel->RemoveChild(Removed->Widget);
        }
        else if (LayerRoot && LayerRoot->GetContent() == Removed->Widget)
        {
            LayerRoot->SetContent(nullptr);
        }
        Removed->Widget->RemoveFromParent();
    }

    if (Index == Stack.Num())
    {
        ShowCurrentTop();
    }

    return Removed;
}

void UNsUILayer::ClearStack()
{
    for (UNsUIWidgetStackElement* Element : Stack)
    {
        if (Element && Element->Widget)
        {
            if (LayerPanel)
            {
                LayerPanel->RemoveChild(Element->Widget);
            }
            else if (LayerRoot && LayerRoot->GetContent() == Element->Widget)
            {
                LayerRoot->SetContent(nullptr);
            }
            Element->Widget->RemoveFromParent();
        }
    }
    Stack.Empty();
    if (LayerRoot)
    {
        LayerRoot->SetContent(nullptr);
    }
}

UNsUIWidgetStackElement* UNsUILayer::Peek()
{
    return Stack.Num() > 0 ? Stack.Last() : nullptr;
}

TArray<UNsUIWidgetStackElement*> UNsUILayer::Unwind()
{
    TArray<UNsUIWidgetStackElement*> Removed;
    while (UNsUIWidgetStackElement* Element = Pop())
    {
        Removed.Add(Element);
    }
    return Removed;
}
