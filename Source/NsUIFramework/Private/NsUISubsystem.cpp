// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUISubsystem.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/Slider.h"
#include "GameFramework/PlayerController.h"
#include "INsUINavigation.h"
#include "Kismet/GameplayStatics.h"
#include "NsUICursor.h"
#include "NsUILayout.h"
#include "NsUILibrary.h"
#include "NsUISettings.h"
#include "NsUIWidget.h"
#include "Stats/Stats.h"

UNsUISubsystem::UNsUISubsystem()
    : CurrentActorIndex(INDEX_NONE)
    , bActorSelectionActive(false)
{
}

void UNsUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    InitializeCommandRegistry();
}

void UNsUISubsystem::Deinitialize()
{
    if (CursorWidget != nullptr)
    {
        CursorWidget->RemoveFromParent();
        CursorWidget = nullptr;
    }

    Super::Deinitialize();
}

UNsUISubsystem* UNsUISubsystem::GetSubsystem()
{
    UNsUISubsystem* ToReturn = nullptr;

    if (GEngine != nullptr)
    {
        // Iterate through World Contexts until a Game World is found
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (const UWorld* const World = Context.World())
            {
                if (World->IsGameWorld() || World->IsPlayInEditor())
                {
                    if (UNsUISubsystem* const Subsystem = UGameInstance::GetSubsystem<UNsUISubsystem>(World->GetGameInstance()))
                    {
                        ToReturn = Subsystem;
                        break;
                    }
                }
            }
        }
    }

    return ToReturn;
}

UUserWidget* UNsUISubsystem::PushWidgetToLayer(const TSoftClassPtr<class UUserWidget>& InWidget, const ENsUILayerType InLayerType)
{
    if (ActiveUILayout != nullptr)
    {
        CurrentScope = ActiveUILayout->PushContentToLayer(InLayerType, InWidget);
        if (CurrentScope != nullptr)
        {
            SetCurrentFocusedWidget(CurrentScope->WidgetScopeFocusedElement);
            return CurrentScope->WidgetScope;
        }
        else
        {
            SetCurrentFocusedWidget(nullptr);
        }
    }

    return nullptr;
}

void UNsUISubsystem::PopWidgetFromLayer(const ENsUILayerType InLayerType)
{
    if (ActiveUILayout != nullptr)
    {
        // get the new top-of-stack (the widget we are restoring/bringing back)
        CurrentScope = ActiveUILayout->PopContentFromLayer(InLayerType);
        if (CurrentScope != nullptr)
        {
            SetCurrentFocusedWidget(CurrentScope->WidgetScopeFocusedElement);
        }
        else
        {
            SetCurrentFocusedWidget(nullptr);
        }
    }
}

void UNsUISubsystem::SetActiveUILayout(const TSoftClassPtr<class UNsUILayout>& InLayout)
{
    APlayerController* const LocalPlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (InLayout != nullptr && LocalPlayerController != nullptr)
    {
        if (ActiveUILayout != nullptr)
        {
            ActiveUILayout->RemoveFromParent();
        }

        ActiveUILayout = CreateWidget<UNsUILayout>(LocalPlayerController->GetWorld(), InLayout.LoadSynchronous());

        if (ActiveUILayout != nullptr)
        {
            ActiveUILayout->SetOwningPlayer(LocalPlayerController);
            ActiveUILayout->AddToViewport();

            if (const UNsUISettings* const Settings = UNsUISettings::GetSettings())
            {
                if (Settings->CursorWidgetClass.IsNull())
                {
                    UE_LOG(LogTemp, Error, TEXT("Failed to load Cursor"));
                    return;
                }

                CursorWidget = CreateWidget<UNsUICursor>(LocalPlayerController, Settings->CursorWidgetClass.LoadSynchronous());
                if (CursorWidget != nullptr && ActiveUILayout->WidgetTree != nullptr)
                {
                    if (UPanelWidget* const RootPanel = Cast<UPanelWidget>(ActiveUILayout->WidgetTree->RootWidget))
                    {
                        RootPanel->AddChild(CursorWidget);
                        CursorWidget->SetVisibility(ESlateVisibility::Visible);
                        UE_LOG(LogTemp, Warning, TEXT("Cursor was created successfully"));
                    }
                }
            }
        }
    }
}

UNsUILayout* UNsUISubsystem::GetActiveUILayout() const
{
    return ActiveUILayout;
}

INsUINavigation* UNsUISubsystem::GetCurrentNavInterface() const
{
    if (CurrentScope->WidgetScopeFocusedElement != nullptr)
    {
        if (CurrentScope->WidgetScopeFocusedElement->GetClass()->ImplementsInterface(UNsUINavigation::StaticClass()))
        {
            return Cast<INsUINavigation>(CurrentScope->WidgetScopeFocusedElement);
        }
    }
    return nullptr;
}

void UNsUISubsystem::InitializeCommandRegistry()
{
    // Button
    // -----------------------------------------------------------------------------
    // Clicked
    ConfirmRegistry.Add(UButton::StaticClass(), [](UWidget* Widget)
    {
        if (const UButton* const Button = Cast<UButton>(Widget))
        {
            Button->OnClicked.Broadcast();
            Button->OnPressed.Broadcast();
        }
    });

    // Slider
    // -----------------------------------------------------------------------------
    // Confirm
    ConfirmRegistry.Add(USlider::StaticClass(), [](UWidget* Widget)
    {
        if (USlider* const Slider = Cast<USlider>(Widget))
        {
            const float NewVal = FMath::Clamp(Slider->GetValue() + Slider->GetStepSize(), Slider->GetMinValue(), Slider->GetMaxValue());
            Slider->SetValue(NewVal);
            Slider->OnValueChanged.Broadcast(NewVal);
        }
    });

    // Text input
    // -------------------------------------------------------------------------
    // Confirm
    ConfirmRegistry.Add(UEditableText::StaticClass(), [](UWidget* Widget)
    {
        if (const UEditableText* const EditableText = Cast<UEditableText>(Widget))
        {
            EditableText->OnTextCommitted.Broadcast(EditableText->GetText(), ETextCommit::OnEnter);
        }
    });
}

void UNsUISubsystem::SetCurrentFocusedWidget(UWidget* const InNewFocus)
{
    CursorWidget->InvalidateLayoutAndVolatility();
    
    if (InNewFocus != nullptr && InNewFocus->GetOwningPlayer() != nullptr)
    {
        // Actually request keyboard focus for that widget:
        InNewFocus->SetUserFocus(InNewFocus->GetOwningPlayer());
        InNewFocus->SetKeyboardFocus();

        if (CursorWidget != nullptr)
        {
            if (const UWorld* const World = GetWorld())
            {
                World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(CursorWidget, &UNsUICursor::MoveToWidget, InNewFocus));
            }
        }
    }
    else
    {
        if (CursorWidget != nullptr)
        {
            CursorWidget->MoveToWidget(nullptr);
        }
    }

    if (CurrentScope != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Current Focus: %s"), CurrentScope->WidgetScopeFocusedElement != nullptr ? *CurrentScope->WidgetScopeFocusedElement->GetName() : TEXT("Null"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("There is no current scope to handle focus"));
    }
}

UWidget* UNsUISubsystem::GetCurrentFocusedWidget() const
{
    return CurrentScope->WidgetScopeFocusedElement;
}

void UNsUISubsystem::HandleNavigate(const FVector2D& Direction)
{
    // Map the 2D vector into a single EUINavigation enum:
    EUINavigation NavEnum = EUINavigation::Invalid;

    if (Direction.X < 0.0f)      NavEnum = EUINavigation::Left;
    else if (Direction.X > 0.0f) NavEnum = EUINavigation::Right;
    else if (Direction.Y < 0.0f) NavEnum = EUINavigation::Up;
    else if (Direction.Y > 0.0f) NavEnum = EUINavigation::Down;

    if (NavEnum == EUINavigation::Invalid)
    {
        return; // no meaningful direction
    }

    if (bActorSelectionActive)
    {
        if (ActorFocusTargets.Num() == 0)
        {
            return;
        }

        const int32 NextIndex = FindBestActorBySpatialSearch(CurrentActorIndex, NavEnum);
        if (NextIndex != CurrentActorIndex)
        {
            CurrentActorIndex = NextIndex;
            UpdateActorCursor();
        }
        return;
    }

    if (CurrentScope->WidgetScopeFocusedElement == nullptr)
    {
        return;
    }

    // First, try the explicit neighbor pointer:
    if (const INsUINavigation* const NavInt = GetCurrentNavInterface())
    {
        UWidget* NextWidget = nullptr;
        switch (NavEnum)
        {
            case EUINavigation::Up:    NextWidget = NavInt->Execute_GetUpNeighbor(CurrentScope->WidgetScopeFocusedElement);    break;
            case EUINavigation::Down:  NextWidget = NavInt->Execute_GetDownNeighbor(CurrentScope->WidgetScopeFocusedElement);  break;
            case EUINavigation::Left:  NextWidget = NavInt->Execute_GetLeftNeighbor(CurrentScope->WidgetScopeFocusedElement);  break;
            case EUINavigation::Right: NextWidget = NavInt->Execute_GetRightNeighbor(CurrentScope->WidgetScopeFocusedElement); break;
            default: return;
        }

        // If the explicit pointer is non‐null, jump to it immediately:
        if (NextWidget != nullptr && NextWidget->IsValidLowLevel() && NextWidget->IsInViewport())
        {
            CurrentScope->WidgetScopeFocusedElement = NextWidget;
            SetCurrentFocusedWidget(NextWidget);
            return;
        }
    }

    // Otherwise, fall back to the “spatial search”:
    UWidget* const SpatialCandidate = FindBestNeighborBySpatialSearch(CurrentScope->WidgetScopeFocusedElement, NavEnum);
    if (SpatialCandidate != nullptr && SpatialCandidate->IsValidLowLevel())
    {
        CurrentScope->WidgetScopeFocusedElement = SpatialCandidate;
        SetCurrentFocusedWidget(SpatialCandidate);
    }
}

bool UNsUISubsystem::HandleConfirm()
{
    // If we’re in actor selection mode, short-circuit to that flow
    if (bActorSelectionActive)
    {
        if (ActorFocusTargets.IsValidIndex(CurrentActorIndex))
        {
            OnActorConfirmed.Broadcast(ActorFocusTargets[CurrentActorIndex].Actor.Get());
        }
        ExitActorSelectionMode();
        return true;
    }

    // Nothing focused -> nothing to do
    UWidget* const Focused = CurrentScope->WidgetScopeFocusedElement;
    if (!Focused)
    {
        return false;
    }

    // Helper: is this widget a viable target for confirm?
    auto IsInteractable = [](const UWidget* W) -> bool
    {
        return IsValid(W) && W->GetIsEnabled() && W->IsVisible();
    };

    // Helper: does this widget match any entry in the confirm registry?
    auto HasRegistryMatch = [this](const UWidget* W) -> bool
    {
        for (const auto& Pair : ConfirmRegistry)
        {
            if (W && W->IsA(Pair.Key))
            {
                return true;
            }
        }
        return false;
    };

    // Helper: execute the matching handler for W (first match in the current registry iteration order)
    auto TryExecuteRegistry = [this](UWidget* W) -> bool
    {
        for (const auto& Pair : ConfirmRegistry)
        {
            if (W && W->IsA(Pair.Key))
            {
                Pair.Value(W);
                return true;
            }
        }
        return false;
    };

    // 1) Direct hit on the focused widget
    if (IsInteractable(Focused) && HasRegistryMatch(Focused))
    {
        if (TryExecuteRegistry(Focused))
        {
            return true;
        }
    }

    // 2) BFS: search *descendants* of the focused widget (nearest-first)
    {
        TQueue<UWidget*> Queue;

        auto EnqueueChild = [&Queue](UWidget* Child)
        {
            if (IsValid(Child)) { Queue.Enqueue(Child); }
        };

        auto EnqueueChildren = [&EnqueueChild](UWidget* W)
        {
            // Dive into nested UUserWidget content
            if (UUserWidget* const UW = Cast<UUserWidget>(W))
            {
                if (UW->WidgetTree && UW->WidgetTree->RootWidget)
                {
                    EnqueueChild(UW->WidgetTree->RootWidget);
                }
            }

            // Panel/content widgets
            if (UPanelWidget* const Panel = Cast<UPanelWidget>(W))
            {
                const int32 Count = Panel->GetChildrenCount();
                for (int32 i = 0; i < Count; ++i)
                {
                    EnqueueChild(Panel->GetChildAt(i));
                }
            }
        };

        EnqueueChildren(Focused);

        UWidget* Curr = nullptr;
        while (Queue.Dequeue(Curr))
        {
            if (IsInteractable(Curr) && HasRegistryMatch(Curr))
            {
                if (TryExecuteRegistry(Curr))
                {
                    return true;
                }
            }
            EnqueueChildren(Curr);
        }
    }

    // 3) As a last local attempt, walk *ancestors* (in case the actionable lives above, e.g. a wrapper button)
    {
        for (UWidget* P = Focused->GetParent(); P; P = P->GetParent())
        {
            if (IsInteractable(P) && HasRegistryMatch(P))
            {
                if (TryExecuteRegistry(P))
                {
                    return true;
                }
            }
        }
    }

    // 4) Fallback to widget-level interface (your existing behavior)
    if (const INsUINavigation* const Nav = GetCurrentNavInterface())
    {
        Nav->Execute_HandleUIConfirm(Focused);
        return true;
    }

    return false;
}

bool UNsUISubsystem::HandleCancel()
{
    // Actor selection mode takes precedence.
    if (bActorSelectionActive)
    {
        if (ActorFocusTargets.IsValidIndex(CurrentActorIndex))
        {
            OnActorCancelled.Broadcast(ActorFocusTargets[CurrentActorIndex].Actor.Get());
        }
        ExitActorSelectionMode();
        return true;
    }

    if (CurrentScope != nullptr)
    {
        
        if (CurrentScope->WidgetScopeFocusedElement == nullptr)
        {
            return false;
        }

        // Helpers
        auto IsInteractable = [](const UWidget* W) -> bool
        {
            return IsValid(W) && W->GetIsEnabled() && W->IsVisible();
        };

        auto TryExecuteCancelRegistry = [this](UWidget* W) -> bool
        {
            for (const auto& Pair : CancelRegistry)
            {
                if (W && W->IsA(Pair.Key))
                {
                    Pair.Value(W);
                    return true;
                }
            }
            return false;
        };

        auto TryExecuteCancelInterface = [](UWidget* Scope) -> bool
        {
            if (Scope->GetClass()->ImplementsInterface(UNsUINavigation::StaticClass()))
            {
                if (const INsUINavigation* const NavigationInterface = Cast<INsUINavigation>(Scope))
                {
                    NavigationInterface->Execute_HandleUICancel(Scope);
                    return true;
                }
            }

            return false;
        };

        // 1) Focused widget first (cheap fast path).
        if (IsInteractable(CurrentScope->WidgetScope) && TryExecuteCancelRegistry(CurrentScope->WidgetScope))
        {
            return true;
        }

        // 2) Bubble up ancestors *within the same layer scope* (Inventory should catch here).
        UUserWidget* const LayerScope = CurrentScope->WidgetScope; // keeps us inside Static/Menu/Modal
        for (UWidget* P = CurrentScope->WidgetScope->GetParent(); P; P = P->GetParent())
        {
            if (!IsInteractable(P))
            {
                continue;
            }

            if (TryExecuteCancelRegistry(P) || TryExecuteCancelInterface(P))
            {
                return true;
            }

            // Stop at the layer content boundary; do not bleed into other layers/layout.
            if (P == LayerScope)
            {
                break;
            }
        }

        // 3) As a last in-layer attempt, give the layer content root a chance (if not already hit).
        if (LayerScope && LayerScope != CurrentScope->WidgetScope)
        {
            if (IsInteractable(LayerScope))
            {
                if (TryExecuteCancelRegistry(LayerScope) || TryExecuteCancelInterface(LayerScope))
                {
                    return true;
                }
            }
        }

        // 4) Final fallback: current focused widget’s own interface (existing behavior).
        if (CurrentScope->WidgetScope != nullptr)
        {
            if (CurrentScope->WidgetScope->GetClass()->ImplementsInterface(UNsUINavigation::StaticClass()))
            {
                if (const INsUINavigation* Nav = Cast<INsUINavigation>(CurrentScope->WidgetScope))
                {
                    Nav->Execute_HandleUICancel(CurrentScope->WidgetScope);
                    return true;
                }
            }
        }
        
    }

    return false;
}

UWidget* UNsUISubsystem::FindBestNeighborBySpatialSearch(UWidget* const InCurrentWidget, const EUINavigation InNavDir) const
{
    if (!InCurrentWidget || !InCurrentWidget->GetOwningPlayer())
    {
        return nullptr;
    }

    const UUserWidget* LayerRoot = UNsUILibrary::FindLayerContentRoot(InCurrentWidget);
    if (!LayerRoot)
    {
        return nullptr;
    }

    TArray<UWidget*> NavigableWidgets; UNsUILibrary::GatherAllNavigableDescendants(LayerRoot, NavigableWidgets);
    NavigableWidgets.Remove(InCurrentWidget);
    if (NavigableWidgets.Num() == 0)
    {
        return InCurrentWidget;
    }

    const FVector2D CurrentCenter = GetWidgetCenterInViewport(InCurrentWidget);

    FVector2D DesiredDirection(0, 0);
    switch (InNavDir)
    {
        case EUINavigation::Up:    DesiredDirection = FVector2D( 0.f,  1.f); break;
        case EUINavigation::Down:  DesiredDirection = FVector2D( 0.f, -1.f); break;
        case EUINavigation::Left:  DesiredDirection = FVector2D(-1.f,  0.f); break;
        case EUINavigation::Right: DesiredDirection = FVector2D( 1.f,  0.f); break;
        default: return InCurrentWidget; // unsupported navigation
    }

    // Gather candidates in desired half-plane
    TArray<TPair<UWidget*, FVector2D>> ValidCandidates;
    ValidCandidates.Reserve(NavigableWidgets.Num());

    for (UWidget* const Candidate : NavigableWidgets)
    {
        if (Candidate == nullptr)
        {
            continue;
        }

        const FVector2D CandidateCenter = GetWidgetCenterInViewport(Candidate);
        const FVector2D Delta = CandidateCenter - CurrentCenter;

        // Only keep those strictly in the desired half-plane
        if (FVector2D::DotProduct(Delta, DesiredDirection) > 0.0f)
        {
            ValidCandidates.Emplace(Candidate, Delta);
        }
    }

    // If none found, relax to consider every candidate so navigation always
    // advances
    // if (ValidCandidates.Num() == 0)
    // {
    //     for (UWidget* const Candidate : NavigableWidgets)
    //     {
    //         if (Candidate == nullptr)
    //         {
    //             continue;
    //         }
    //         const FVector2D CandidateCenter = GetWidgetCenterInViewport(Candidate);
    //         ValidCandidates.Emplace(Candidate, CandidateCenter - CurrentCenter);
    //     }
    // }

    if (ValidCandidates.Num() == 0)
    {
        // No other widgets at all
        return InCurrentWidget;
    }

    // Sort by (1) angle closeness, then (2) distance
    ValidCandidates.Sort([&](auto& InA, auto& InB)
    {
        const float CosA = FVector2D::DotProduct(InA.Value.GetSafeNormal(), DesiredDirection);
        const float CosB = FVector2D::DotProduct(InB.Value.GetSafeNormal(), DesiredDirection);
        if (!FMath::IsNearlyEqual(CosA, CosB))
        {
            return CosA > CosB;
        }
        return InA.Value.SizeSquared() < InB.Value.SizeSquared();
    });

    // Best candidate is front of the list
    return ValidCandidates[0].Key;
}

int32 UNsUISubsystem::FindBestActorBySpatialSearch(int32 CurrentIndex, EUINavigation InNavDir) const
{
    if (!ActorFocusTargets.IsValidIndex(CurrentIndex))
    {
        return CurrentIndex;
    }

    const FVector2D CurrentCenter = GetActorScreenCenter(ActorFocusTargets[CurrentIndex]);

    FVector2D DesiredDirection(0.f, 0.f);
    switch (InNavDir)
    {
        case EUINavigation::Up:    DesiredDirection = FVector2D( 0.f, -1.f); break;
        case EUINavigation::Down:  DesiredDirection = FVector2D( 0.f,  1.f); break;
        case EUINavigation::Left:  DesiredDirection = FVector2D(-1.f,  0.f); break;
        case EUINavigation::Right: DesiredDirection = FVector2D( 1.f,  0.f); break;
        default: return CurrentIndex;
    }

    // Track best candidate. We first search only the half-plane in the
    // requested direction. If none are found, we relax to the closest
    // angular match so navigation always advances somewhere.
    int32 BestIndex = CurrentIndex;
    float BestScore = -FLT_MAX;
    bool bFoundForward = false;

    for (int32 i = 0; i < ActorFocusTargets.Num(); ++i)
    {
        if (i == CurrentIndex)
        {
            continue;
        }

        const FVector2D Delta = GetActorScreenCenter(ActorFocusTargets[i]) - CurrentCenter;
        if (Delta.IsNearlyZero())
        {
            continue;
        }

        const FVector2D Dir = Delta.GetSafeNormal();
        const float Dot = FVector2D::DotProduct(Dir, DesiredDirection);
        const float Score = Dot - 0.0001f * Delta.SizeSquared();

        if (Dot > 0.f)
        {
            if (!bFoundForward || Score > BestScore)
            {
                bFoundForward = true;
                BestScore = Score;
                BestIndex = i;
            }
        }
        else if (!bFoundForward && Score > BestScore)
        {
            BestScore = Score;
            BestIndex = i;
        }
    }

    return BestIndex;
}

FVector2D UNsUISubsystem::GetWidgetCenterInViewport(const UWidget* const InWidget) const
{
    // Since UUserWidget::GetCachedGeometry() is protected, we have this helper
    // inside our C++ subclass so we can call it directly. It returns the center
    // in screen coords.
    if (InWidget == nullptr)
    {
        return FVector2D::ZeroVector;
    }

    // Ask the widget for its cached geometry.  Since this is a method on UWidget
    // (protected), and UUserWidget inherits from UWidget, we can call it directly
    // from here.
    const FGeometry Geo = InWidget->GetCachedGeometry();

    // We want the widget’s center in “absolute” (viewport) space.  We can do:
    // Geo.GetLocalToAbsoluteTransform().TransformPoint(LocalCenter) where
    // LocalCenter = (WidgetSize / 2, WidgetSize / 2).
    const FVector2D LocalSize = Geo.GetLocalSize();
    const FVector2D LocalCenter = LocalSize * 0.5f;
    const FVector2D AbsoluteCenter = Geo.LocalToAbsolute(LocalCenter);

    return AbsoluteCenter;
}

void UNsUISubsystem::HandleToggle()
{
    if (bActorSelectionActive &&
        ActorFocusTargets.IsValidIndex(CurrentActorIndex))
    {
        OnActorToggled.Broadcast(ActorFocusTargets[CurrentActorIndex].Actor.Get());
    }
}

void UNsUISubsystem::EnterActorSelectionMode()
{
    bActorSelectionActive = true;
    CurrentActorIndex = ActorFocusTargets.Num() > 0 ? 0 : INDEX_NONE;
    UpdateActorCursor();
}

void UNsUISubsystem::ExitActorSelectionMode()
{
    bActorSelectionActive = false;
    CurrentActorIndex = INDEX_NONE;
    if (CursorWidget)
    {
        CursorWidget->MoveToWidget(nullptr);
    }
}

int32 UNsUISubsystem::RegisterActorFocus(AActor* Actor, const FBox2D& ScreenBounds)
{
    FNsUIActorFocusInfo Info;
    Info.Actor = Actor;
    Info.ScreenBounds = ScreenBounds;
    return ActorFocusTargets.Add(Info);
}

void UNsUISubsystem::UnregisterActorFocus(AActor* Actor)
{
    ActorFocusTargets.RemoveAll([Actor](const FNsUIActorFocusInfo& Info)
    {
        return Info.Actor == Actor;
    });
    
    if (CurrentActorIndex >= ActorFocusTargets.Num())
    {
        CurrentActorIndex = ActorFocusTargets.Num() - 1;
    }

    UpdateActorCursor();
}

void UNsUISubsystem::ClearActorFocusTargets()
{
    ActorFocusTargets.Reset();
    CurrentActorIndex = INDEX_NONE;
    if (CursorWidget)
    {
        CursorWidget->MoveToWidget(nullptr);
    }
}

void UNsUISubsystem::UpdateActorCursor()
{
    if (!bActorSelectionActive || !CursorWidget)
    {
        return;
    }

    if (ActorFocusTargets.IsValidIndex(CurrentActorIndex))
    {
        const FVector2D Center = GetActorScreenCenter(ActorFocusTargets[CurrentActorIndex]);
        CursorWidget->MoveToScreenPosition(Center);
    }
    else
    {
        CursorWidget->MoveToWidget(nullptr);
    }
}

UNsUIWidgetStackElement* UNsUISubsystem::GetCurrentScope()
{
    return CurrentScope;
}

FVector2D UNsUISubsystem::GetActorScreenCenter(const FNsUIActorFocusInfo& Info) const
{
    return (Info.ScreenBounds.Min + Info.ScreenBounds.Max) * 0.5f;
}

void UNsUISubsystem::Tick(float DeltaTime)
{
    if (!bActorSelectionActive)
    {
        return;
    }

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        for (FNsUIActorFocusInfo& Info : ActorFocusTargets)
        {
            if (AActor* Actor = Info.Actor.Get())
            {
                const FBox Bounds = Actor->GetComponentsBoundingBox();
                FVector2D Min, Max;
                PC->ProjectWorldLocationToScreen(Bounds.Min, Min, true);
                PC->ProjectWorldLocationToScreen(Bounds.Max, Max, true);
                Info.ScreenBounds = FBox2D(Min, Max);
            }
        }
    }

    UpdateActorCursor();
}

TStatId UNsUISubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UNsUISubsystem, STATGROUP_Tickables);
}
