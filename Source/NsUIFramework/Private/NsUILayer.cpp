// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUILayer.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/PanelWidget.h"
#include "NsUIWidget.h"

void UNsUILayer::NativeConstruct()
{
    Super::NativeConstruct();
    Stack.Reset();
}

UNsUIWidgetStackElement* UNsUILayer::CreateStackElement(UUserWidget* Widget, bool bCollapsedBelow) const
{
    if (!Widget)
    {
        return nullptr;
    }

    UNsUIWidgetStackElement* Element = NewObject<UNsUIWidgetStackElement>(this);
    Element->Widget = Widget;
    Element->Layer = ENsUILayerType::None;
    Element->bCollapsedEntryBelow = bCollapsedBelow;
    return Element;
}

void UNsUILayer::AttachToLayer(UUserWidget* Widget) const
{
    if (!Widget)
    {
        return;
    }

    if (LayerPanel)
    {
        LayerPanel->AddChild(Widget);
    }
    else if (LayerRoot)
    {
        LayerRoot->SetContent(Widget);
    }
}

void UNsUILayer::DetachFromLayer(UUserWidget* Widget) const
{
    if (!Widget)
    {
        return;
    }

    if (LayerPanel)
    {
        LayerPanel->RemoveChild(Widget);
    }
    else if (LayerRoot && LayerRoot->GetContent() == Widget)
    {
        LayerRoot->SetContent(nullptr);
    }

    Widget->RemoveFromParent();
}

void UNsUILayer::NotifyWidgetPushed(UUserWidget* Widget) const
{
    if (INsUINavigation* Navigation = Cast<INsUINavigation>(Widget))
    {
        Navigation->Execute_OnWidgetPushed(Widget);
    }
}

void UNsUILayer::ShowTopAndRestoreFocus()
{
    if (UNsUIWidgetStackElement* Top = Peek())
    {
        if (UUserWidget* Widget = Top->Widget.Get())
        {
            if (LayerPanel && Widget->GetParent() != LayerPanel)
            {
                LayerPanel->AddChild(Widget);
            }
            else if (LayerRoot && LayerRoot->GetContent() != Widget)
            {
                LayerRoot->SetContent(Widget);
            }

            Widget->SetVisibility(ESlateVisibility::Visible);

            if (INsUINavigation* Navigation = Cast<INsUINavigation>(Widget))
            {
                if (UWidget* FocusTarget = Navigation->Execute_GetInitialFocusWidget(Widget))
                {
                    FocusTarget->SetKeyboardFocus();
                }
                else
                {
                    Widget->SetKeyboardFocus();
                }
            }
            else
            {
                Widget->SetKeyboardFocus();
            }
        }
    }
}

UNsUIWidgetStackElement* UNsUILayer::PushClass(const TSoftClassPtr<UUserWidget>& InClass, bool bCollapsePrevious)
{
    APlayerController* OwningPlayer = GetOwningPlayer();
    if (!OwningPlayer)
    {
        return nullptr;
    }

    UClass* WidgetClass = InClass.LoadSynchronous();
    if (!WidgetClass)
    {
        return nullptr;
    }

    UUserWidget* NewWidget = CreateWidget<UUserWidget>(OwningPlayer, WidgetClass);
    return PushWidget(NewWidget, bCollapsePrevious);
}

UNsUIWidgetStackElement* UNsUILayer::PushWidget(UUserWidget* InWidget, bool bCollapsePrevious)
{
    if (!InWidget)
    {
        return nullptr;
    }

    if (!LayerRoot && !LayerPanel)
    {
        return nullptr;
    }

    if (bCollapsePrevious)
    {
        if (UNsUIWidgetStackElement* Top = Peek())
        {
            if (Top->Widget)
            {
                Top->Widget->SetVisibility(ESlateVisibility::Collapsed);
            }
        }
    }

    AttachToLayer(InWidget);

    UNsUIWidgetStackElement* Element = CreateStackElement(InWidget, bCollapsePrevious);
    Stack.Add(Element);

    NotifyWidgetPushed(InWidget);
    ShowTopAndRestoreFocus();

    return Element;
}

UNsUIWidgetStackElement* UNsUILayer::ReplaceTopWithClass(const TSoftClassPtr<UUserWidget>& InClass, bool bCollapsePrevious)
{
    Pop();
    return PushClass(InClass, bCollapsePrevious);
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
        DetachFromLayer(Removed->Widget);
    }

    ShowTopAndRestoreFocus();
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
        DetachFromLayer(Removed->Widget);
    }

    if (Index == Stack.Num())
    {
        ShowTopAndRestoreFocus();
    }

    return Removed;
}

void UNsUILayer::ClearStack()
{
    for (UNsUIWidgetStackElement* Element : Stack)
    {
        if (Element && Element->Widget)
        {
            DetachFromLayer(Element->Widget);
        }
    }

    Stack.Empty();

    if (LayerRoot)
    {
        LayerRoot->SetContent(nullptr);
    }
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

UNsUIWidgetStackElement* UNsUILayer::Peek() const
{
    return Stack.Num() > 0 ? Stack.Last() : nullptr;
}
