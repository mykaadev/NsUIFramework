// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUILibrary.h"
#include "Blueprint/WidgetTree.h"
#include "INsUINavigation.h"
#include "NsUILayout.h"
#include "NsUISubsystem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

UUserWidget* UNsUILibrary::AddWidgetToLayer(const TSoftClassPtr<class UUserWidget>& InWidget, const ENsUILayerType InLayerType)
{
    if (UNsUISubsystem* const UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        return UISubsystem->PushWidgetToLayer(InWidget, InLayerType);
    }

    return nullptr;
}

void UNsUILibrary::PopWidgetFromLayer(const ENsUILayerType InLayerType)
{
    if (UNsUISubsystem* const UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        UISubsystem->PopWidgetFromLayer(InLayerType);
    }
}

UUserWidget* UNsUILibrary::FindLayerContentRoot(UWidget* InWidget)
{
    if (!IsValid(InWidget)) return nullptr;

    // 1) Climb the visual parent chain to the local root panel inside the innermost user widget
    UWidget* Visual = InWidget;
    while (UPanelWidget* P = Visual->GetParent())
    {
        Visual = P;
    }

    // 2) That panel belongs to some innermost UUserWidget (e.g., Item)
    UUserWidget* InnerMostUW = Visual->GetTypedOuter<UUserWidget>();
    if (!InnerMostUW) return nullptr;

    // 3) Climb outward across nested UUserWidgets until the parent panel lives under a NsUILayer
    UUserWidget* OutermostWithinLayer = InnerMostUW;
    for (;;)
    {
        UPanelWidget* ParentPanel = OutermostWithinLayer->GetParent();
        if (!ParentPanel) break;

        // When the parent panel is in a UNsUILayer's WidgetTree, we've reached the layer boundary
        if (ParentPanel->GetTypedOuter<UNsUILayer>() != nullptr)
        {
            break; // OutermostWithinLayer is the layer's top-of-stack content widget
        }

        // Otherwise step out one level: panel belongs to another UUserWidget (e.g., Inventory)
        if (UUserWidget* NextUW = ParentPanel->GetTypedOuter<UUserWidget>())
        {
            OutermostWithinLayer = NextUW;
        }
        else
        {
            break;
        }
    }

    return OutermostWithinLayer;
}

static bool CanWidgetReceiveFocus(const UWidget* Widget)
{
    if (!Widget)
    {
        return false;
    }

    // Make sure the UWidget has already constructed its Slate counterpart
    const TSharedPtr<SWidget> SlateWidget = Widget->GetCachedWidget();
    if (!SlateWidget.IsValid())
    {
        return false;
    }

    // Slate’s SupportsKeyboardFocus tells you if it can ever receive focus
    return SlateWidget->SupportsKeyboardFocus();
}

void UNsUILibrary::GatherAllNavigableDescendants(const UUserWidget* const InRootWidget, TArray<UWidget*>& InOutNavigables)
{
    if (!InRootWidget || !InRootWidget->WidgetTree)
    {
        return;
    }

    TArray<UWidget*> AllWidgets;
    InRootWidget->WidgetTree->GetAllWidgets(AllWidgets);

    for (UWidget* Widget : AllWidgets)
    {
        if (!Widget || !Widget->IsVisible() || !Widget->GetIsEnabled())
        {
            continue;
        }

        // 1) Engine‐defined focusable?
        const bool bSlateFocusable = CanWidgetReceiveFocus(Widget);

        // 2) Your custom interface?
        const bool bCustomNav = Widget->GetClass()->ImplementsInterface(UNsUINavigation::StaticClass());

        if (bSlateFocusable || bCustomNav)
        {
            InOutNavigables.Emplace(Widget);
        }
    }
}

void UNsUILibrary::RegisterActorsForSelection(
    const TArray<AActor*>& Actors,
    const FNsUIActorDelegate::FDelegate& OnConfirm,
    const FNsUIActorDelegate::FDelegate& OnCancel,
    const FNsUIActorDelegate::FDelegate& OnToggle)
{
    if (UNsUISubsystem* const UISubsystem = UNsUISubsystem::GetSubsystem())
    {
        // Reset any previous selection state
        UISubsystem->ClearActorFocusTargets();
        UISubsystem->OnActorConfirmed.Clear();
        UISubsystem->OnActorCancelled.Clear();
        UISubsystem->OnActorToggled.Clear();

        UWorld* World = UISubsystem->GetWorld();
        APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
        if (!PC)
        {
            return;
        }

        for (AActor* Actor : Actors)
        {
            if (!Actor)
            {
                continue;
            }

            const FBox Bounds = Actor->GetComponentsBoundingBox(true);
            FVector Corners[8];
            Bounds.GetVertices(Corners);

            FVector2D Min(TNumericLimits<float>::Max(), TNumericLimits<float>::Max());
            FVector2D Max(TNumericLimits<float>::Lowest(), TNumericLimits<float>::Lowest());

            for (const FVector& Corner : Corners)
            {
                FVector2D ScreenPos;
                if (PC->ProjectWorldLocationToScreen(Corner, ScreenPos, true))
                {
                    Min.X = FMath::Min(Min.X, ScreenPos.X);
                    Min.Y = FMath::Min(Min.Y, ScreenPos.Y);
                    Max.X = FMath::Max(Max.X, ScreenPos.X);
                    Max.Y = FMath::Max(Max.Y, ScreenPos.Y);
                }
            }

            if (Min.X <= Max.X && Min.Y <= Max.Y)
            {
                UISubsystem->RegisterActorFocus(Actor, FBox2D(Min, Max));
            }
        }

        if (OnConfirm.IsBound())
        {
            UISubsystem->OnActorConfirmed.Add(OnConfirm);
        }
        if (OnCancel.IsBound())
        {
            UISubsystem->OnActorCancelled.Add(OnCancel);
        }
        if (OnToggle.IsBound())
        {
            UISubsystem->OnActorToggled.Add(OnToggle);
        }
    }
}
