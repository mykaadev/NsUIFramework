// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUILayer.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/PanelWidget.h"
#include "INsUINavigation.h"
#include "NsUILibrary.h"
#include "NsUIWidget.h"

TSharedRef<SWidget> UNsUILayer::RebuildWidget()
{
    if (WidgetTree == nullptr)
    {
        WidgetTree = NewObject<UWidgetTree>(this, UWidgetTree::StaticClass(), TEXT("WidgetTree"));
    }

    if (WidgetTree->RootWidget == nullptr)
    {
        UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("LayerRoot"));
        RootOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        WidgetTree->RootWidget = RootOverlay;
    }

    LayerRoot = Cast<UPanelWidget>(WidgetTree->RootWidget);
    return Super::RebuildWidget();
}

bool UNsUILayer::PushWidget(UNsUIWidget* Widget, UWidget* PreviousFocus)
{
    if (!IsValid(Widget) || !IsValid(LayerRoot))
    {
        return false;
    }

    if (UPanelWidget* Panel = Cast<UPanelWidget>(LayerRoot))
    {
        if (UNsUIWidget* Top = PeekWidget())
        {
            Top->SetVisibility(ESlateVisibility::Collapsed);
        }

        FocusHistory.Add(PreviousFocus);
        WidgetStack.Add(Widget);

        Panel->ClearChildren();
        Panel->AddChild(Widget);
        Widget->SetVisibility(ESlateVisibility::Visible);

        if (Widget->GetClass()->ImplementsInterface(UNsUINavigation::StaticClass()))
        {
            Widget->Execute_OnWidgetActivated(Widget);
        }

        if (UWidget* InitialFocus = FindInitialFocus(Widget))
        {
            InitialFocus->SetKeyboardFocus();
            InitialFocus->SetUserFocus(GetOwningPlayer());
        }
        return true;
    }

    return false;
}

UNsUIWidget* UNsUILayer::PopWidget()
{
    if (!WidgetStack.Num())
    {
        return nullptr;
    }

    UNsUIWidget* Removed = WidgetStack.Pop().Get();
    if (Removed)
    {
        if (Removed->GetClass()->ImplementsInterface(UNsUINavigation::StaticClass()))
        {
            Removed->Execute_OnWidgetDeactivated(Removed);
        }
        Removed->RemoveFromParent();
    }

    if (LayerRoot)
    {
        LayerRoot->ClearChildren();
    }

    if (UNsUIWidget* NewTop = PeekWidget())
    {
        NewTop->SetVisibility(ESlateVisibility::Visible);
        if (LayerRoot)
        {
            LayerRoot->AddChild(NewTop);
        }
    }

    RestoreLastFocus();
    return Removed;
}

void UNsUILayer::PopToRoot()
{
    while (WidgetStack.Num() > 1)
    {
        PopWidget();
    }
}

UNsUIWidget* UNsUILayer::PeekWidget() const
{
    return WidgetStack.IsEmpty() ? nullptr : WidgetStack.Last().Get();
}

UWidget* UNsUILayer::RestoreLastFocus()
{
    while (FocusHistory.Num() > 0)
    {
        if (UWidget* Candidate = FocusHistory.Pop().Get())
        {
            if (Candidate->GetIsEnabled() && Candidate->IsVisible())
            {
                Candidate->SetKeyboardFocus();
                Candidate->SetUserFocus(GetOwningPlayer());
                return Candidate;
            }
        }
    }

    if (UNsUIWidget* Top = PeekWidget())
    {
        if (UWidget* Initial = FindInitialFocus(Top))
        {
            Initial->SetKeyboardFocus();
            Initial->SetUserFocus(GetOwningPlayer());
            return Initial;
        }
    }

    return nullptr;
}

UWidget* UNsUILayer::FindInitialFocus(UNsUIWidget* Widget) const
{
    if (Widget == nullptr)
    {
        return nullptr;
    }

    if (Widget->GetClass()->ImplementsInterface(UNsUINavigation::StaticClass()))
    {
        if (UWidget* Explicit = Widget->Execute_GetInitialFocusWidget(Widget))
        {
            return Explicit;
        }
    }

    return UNsUILibrary::FindFirstFocusableDescendant(Widget);
}
