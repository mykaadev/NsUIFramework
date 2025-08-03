// Copyright (C) 2024 mykaa. All rights reserved.

#include "NsUILibrary.h"
#include "Blueprint/WidgetTree.h"
#include "INsUINavigation.h"
#include "NsUILayout.h"
#include "NsUISubsystem.h"
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

UUserWidget* UNsUILibrary::FindRootUserWidget(UWidget* const InWidget)
{
    if (InWidget == nullptr)
    {
        return nullptr;
    }

    UUserWidget* LastFound = nullptr;
    UObject* Outer = InWidget;

    // Walk up until we run out of outers.
    while (Outer != nullptr)
    {
        if (UUserWidget* const UserWidget = Cast<UUserWidget>(Outer))
        {
            // Record each UUserWidget we see
            LastFound = UserWidget;
        }

        Outer = Outer->GetOuter();
    }

    // 'LastFound' is the topmost UUserWidget
    return LastFound;
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

void UNsUILibrary::RegisterActorsForSelection(const TArray<AActor*>& Actors,
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

        UISubsystem->EnterActorSelectionMode();
    }
}
