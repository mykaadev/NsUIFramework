// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUILayer.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/PanelWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "NsUIWidget.h"

void UNsUILayer::NativeConstruct()
{
    Super::NativeConstruct();
    Stack.Reset();
    PreviousFocusTarget.Reset();
}

UNsUIWidgetStackElement* UNsUILayer::CreateStackElement(UUserWidget* Widget, const FNsUILayerTransitionRequest& Options) const
{
    if (!Widget)
    {
        return nullptr;
    }

    UNsUIWidgetStackElement* Element = NewObject<UNsUIWidgetStackElement>(this);
    Element->Widget = Widget;
    Element->Layer = ENsUILayerType::None;
    Element->bCollapsedEntryBelow = Options.bCollapsePrevious && !Options.bKeepPreviousVisible;
    Element->StableId = Options.StableId.IsValid() ? Options.StableId : FGuid::NewGuid();
    Element->Tags = Options.Tags;
    Element->Lifetime = Options.Lifetime;
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

void UNsUILayer::TrackFocusBeforeTransition()
{
    if (FSlateApplication::IsInitialized())
    {
        PreviousFocusTarget = FSlateApplication::Get().GetKeyboardFocusedWidget();
    }
}

void UNsUILayer::RestorePreviousFocus()
{
    if (PreviousFocusTarget.IsValid())
    {
        PreviousFocusTarget->SetUserFocus(GetOwningPlayer(), EFocusCause::SetDirectly);
    }
}

void UNsUILayer::CollapsePreviousIfNeeded(bool bCollapsePrevious) const
{
    if (!bCollapsePrevious)
    {
        return;
    }

    if (UNsUIWidgetStackElement* Top = Peek())
    {
        if (UUserWidget* TopWidget = Top->Widget.Get())
        {
            TopWidget->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void UNsUILayer::ShowTopAndRestoreFocus(ENsUIFocusRule FocusRule)
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

            if (FocusRule == ENsUIFocusRule::None)
            {
                return;
            }

            if (FocusRule == ENsUIFocusRule::RestorePrevious)
            {
                RestorePreviousFocus();
                return;
            }

            if (INsUINavigation* Navigation = Cast<INsUINavigation>(Widget))
            {
                if (UWidget* FocusTarget = Navigation->Execute_GetInitialFocusWidget(Widget))
                {
                    FocusTarget->SetKeyboardFocus();
                    return;
                }
            }

            if (FocusRule == ENsUIFocusRule::Auto && PreviousFocusTarget.IsValid())
            {
                RestorePreviousFocus();
            }
            else
            {
                Widget->SetKeyboardFocus();
            }
        }
    }
}

UNsUIWidgetStackElement* UNsUILayer::PushClass(const TSoftClassPtr<UUserWidget>& InClass, const FNsUILayerTransitionRequest& Options)
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
    return PushWidget(NewWidget, Options);
}

UNsUIWidgetStackElement* UNsUILayer::PushWidget(UUserWidget* InWidget, const FNsUILayerTransitionRequest& Options)
{
    if (!InWidget)
    {
        return nullptr;
    }

    if (!LayerRoot && !LayerPanel)
    {
        return nullptr;
    }

    TrackFocusBeforeTransition();
    CollapsePreviousIfNeeded(Options.bCollapsePrevious && !Options.bKeepPreviousVisible);

    AttachToLayer(InWidget);

    UNsUIWidgetStackElement* Element = CreateStackElement(InWidget, Options);
    Stack.Add(Element);

    NotifyWidgetPushed(InWidget);
    ShowTopAndRestoreFocus(Options.FocusRule);

    return Element;
}

UNsUIWidgetStackElement* UNsUILayer::ReplaceTopWithClass(const TSoftClassPtr<UUserWidget>& InClass, const FNsUILayerTransitionRequest& Options)
{
    Pop();
    return PushClass(InClass, Options);
}

UNsUIWidgetStackElement* UNsUILayer::Pop()
{
    if (Stack.Num() == 0)
    {
        return nullptr;
    }

    UNsUIWidgetStackElement* Removed = Stack.Last();
    if (Removed && Removed->Lifetime == ENsUIWidgetLifetime::Pinned)
    {
        return nullptr;
    }

    TrackFocusBeforeTransition();

    Removed = Stack.Pop();
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
    if (Removed && Removed->Lifetime == ENsUIWidgetLifetime::Pinned)
    {
        return nullptr;
    }

    TrackFocusBeforeTransition();
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

int32 UNsUILayer::RemoveWidgetsWithTag(FName Tag)
{
    if (Tag.IsNone())
    {
        return 0;
    }

    TrackFocusBeforeTransition();

    int32 RemovedCount = 0;
    for (int32 Index = Stack.Num() - 1; Index >= 0; --Index)
    {
        if (UNsUIWidgetStackElement* Element = Stack[Index])
        {
            if (Element->Tags.Contains(Tag))
            {
                if (Element->Lifetime != ENsUIWidgetLifetime::Pinned)
                {
                    if (Element->Widget)
                    {
                        DetachFromLayer(Element->Widget);
                    }
                    Stack.RemoveAt(Index);
                    ++RemovedCount;
                }
            }
        }
    }

    ShowTopAndRestoreFocus();
    return RemovedCount;
}

UNsUIWidgetStackElement* UNsUILayer::RemoveByStableId(const FGuid& StableId)
{
    if (!StableId.IsValid())
    {
        return nullptr;
    }

    const int32 Index = Stack.IndexOfByPredicate([StableId](const UNsUIWidgetStackElement* Element)
    {
        return Element && Element->StableId == StableId;
    });

    if (Index == INDEX_NONE)
    {
        return nullptr;
    }

    UNsUIWidgetStackElement* Removed = Stack[Index];
    if (Removed && Removed->Lifetime == ENsUIWidgetLifetime::Pinned)
    {
        return nullptr;
    }

    TrackFocusBeforeTransition();
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
    TrackFocusBeforeTransition();
    for (UNsUIWidgetStackElement* Element : Stack)
    {
        if (Element && Element->Widget && Element->Lifetime != ENsUIWidgetLifetime::Pinned)
        {
            DetachFromLayer(Element->Widget);
        }
    }

    Stack.Empty();

    if (LayerRoot)
    {
        LayerRoot->SetContent(nullptr);
    }

    RestorePreviousFocus();
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
