// Copyright (C) 2024 mykaa. All rights reserved.

#include "NsUISubsystem.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/Slider.h"
#include "INsUINavigation.h"
#include "Kismet/GameplayStatics.h"
#include "NsUICursor.h"
#include "NsUILayout.h"
#include "NsUILibrary.h"

UNsUISubsystem::UNsUISubsystem()
{
}

void UNsUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    InitializeCommandRegistry();

    if (CursorWidgetClass != nullptr)
    {
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
        {
            CursorWidget = CreateWidget<UNsUICursor>(PC, CursorWidgetClass.LoadSynchronous());
            if (CursorWidget)
            {
                CursorWidget->AddToViewport();
                CursorWidget->SetVisibility(ESlateVisibility::Hidden);
            }
        }
    }
}

void UNsUISubsystem::Deinitialize()
{
    if (CursorWidget)
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
        if (UUserWidget* const NewWidget = ActiveUILayout->PushContentToLayer(InLayerType, InWidget))
        {
            if (NewWidget->GetClass()->ImplementsInterface(UNsUINavigation::StaticClass()))
            {
                if (const INsUINavigation* const NavigationInterface = Cast<INsUINavigation>(NewWidget))
                {
                    NavigationInterface->Execute_OnWidgetPushed(NewWidget);
                }
            }

            return NewWidget;
        }
    }

    return nullptr;
}

void UNsUISubsystem::PopWidgetFromLayer(const ENsUILayerType InLayerType)
{
    if (ActiveUILayout != nullptr)
    {
        UUserWidget* const TopWidgetInStack = ActiveUILayout->PopContentFromLayer(InLayerType);
        if (TopWidgetInStack != nullptr && TopWidgetInStack->GetClass()->ImplementsInterface(UNsUINavigation::StaticClass()))
        {
            SetCurrentFocusedWidget(TopWidgetInStack);
        }
        else
        {
            // no navigable widget left → clear focus
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
        }
    }
}

UNsUILayout* UNsUISubsystem::GetActiveUILayout() const
{
    return ActiveUILayout;
}

INsUINavigation* UNsUISubsystem::GetCurrentNavInterface() const
{
    if (CurrentFocusedWidget.IsValid())
    {
        if (CurrentFocusedWidget->GetClass()->ImplementsInterface(UNsUINavigation::StaticClass()))
        {
            return Cast<INsUINavigation>(CurrentFocusedWidget.Get());
        }
    }
    return nullptr;
}

void UNsUISubsystem::InitializeCommandRegistry()
{
    // Button -----------------------------------------------------------------------------
    // Clicked
    ConfirmRegistry.Add(UButton::StaticClass(),[](UWidget* Widget)
    {
        if (auto Button = Cast<UButton>(Widget))
        {
            Button->OnClicked.Broadcast();
            Button->OnPressed.Broadcast();
        }
    });

    // Slider -----------------------------------------------------------------------------
    // Confirm
    ConfirmRegistry.Add(USlider::StaticClass(), [](UWidget* Widget)
    {
        if (auto Slider = Cast<USlider>(Widget))
        {
            const float NewVal = FMath::Clamp(Slider->GetValue() + Slider->GetStepSize(), Slider->GetMinValue(), Slider->GetMaxValue());
            Slider->SetValue(NewVal);
            Slider->OnValueChanged.Broadcast(NewVal);
        }
    });

    // Text input -------------------------------------------------------------------------
    // Confirm
    ConfirmRegistry.Add(UEditableText::StaticClass(), [](UWidget* Widget)
    {
        if (auto EditableText = Cast<UEditableText>(Widget))
        {
            EditableText->OnTextCommitted.Broadcast(EditableText->GetText(), ETextCommit::OnEnter);
        }
    });
}

void UNsUISubsystem::SetCurrentFocusedWidget(UWidget* const InNewFocus)
{
    if (CurrentFocusedWidget != InNewFocus)
    {
        PreviousFocusedWidget = CurrentFocusedWidget;
    }

    if (InNewFocus != nullptr && InNewFocus->GetOwningPlayer())
    {
        CurrentFocusedWidget = InNewFocus;

        // Actually request keyboard focus for that widget:
        InNewFocus->SetUserFocus(InNewFocus->GetOwningPlayer());
        InNewFocus->SetKeyboardFocus();

        if (CursorWidget)
        {
            CursorWidget->MoveToWidget(InNewFocus);
        }
    }
    else
    {
        CurrentFocusedWidget = nullptr;

        if (CursorWidget)
        {
            CursorWidget->MoveToWidget(nullptr);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Previous Focus: %s, New Focus: %s")
        , PreviousFocusedWidget.IsValid() ? *PreviousFocusedWidget.Get()->GetName() : TEXT("Unknown")
        , CurrentFocusedWidget.IsValid() ? *CurrentFocusedWidget.Get()->GetName() : TEXT("Unknown"));
}

void UNsUISubsystem::HandleNavigate(const FVector2D& Direction)
{
    if (!CurrentFocusedWidget.IsValid())
    {
        return;
    }

    // Map the 2D vector into a single EUINavigation enum:
    EUINavigation NavEnum =                   EUINavigation::Invalid;
    if (Direction.X <  0.0f)        NavEnum = EUINavigation::Left;
    else if (Direction.X >  0.0f)   NavEnum = EUINavigation::Right;
    else if (Direction.Y <  0.0f)   NavEnum = EUINavigation::Up;
    else if (Direction.Y >  0.0f)   NavEnum = EUINavigation::Down;

    if (NavEnum == EUINavigation::Invalid)
    {
        return; // no meaningful direction
    }

    // First, try the explicit neighbor pointer:
    if (const INsUINavigation* const NavInt = GetCurrentNavInterface())
    {
        UWidget* NextWidget = nullptr;
        switch (NavEnum)
        {
            case EUINavigation::Up:    NextWidget = NavInt->Execute_GetUpNeighbor(CurrentFocusedWidget.Get());    break;
            case EUINavigation::Down:  NextWidget = NavInt->Execute_GetDownNeighbor(CurrentFocusedWidget.Get());  break;
            case EUINavigation::Left:  NextWidget = NavInt->Execute_GetLeftNeighbor(CurrentFocusedWidget.Get());  break;
            case EUINavigation::Right: NextWidget = NavInt->Execute_GetRightNeighbor(CurrentFocusedWidget.Get()); break;
            default: return;
        }

        // If the explicit pointer is non‐null, jump to it immediately:
        if (NextWidget != nullptr && NextWidget->IsValidLowLevel() && NextWidget->IsInViewport())
        {
            if (APlayerController* const PlayerController = NextWidget->GetOwningPlayer())
            {
                NextWidget->SetUserFocus(PlayerController);
                NextWidget->SetKeyboardFocus();
                CurrentFocusedWidget = NextWidget;
                return;
            }
        }
    }

        // Otherwise, fall back to the “spatial search”:
        UWidget* const SpatialCandidate = FindBestNeighborBySpatialSearch(CurrentFocusedWidget.Get(), NavEnum);
        if (SpatialCandidate != nullptr && SpatialCandidate->IsValidLowLevel())
        {
            SetCurrentFocusedWidget(SpatialCandidate);
        }
}

bool UNsUISubsystem::HandleConfirm()
{
    if (UWidget* Widget = CurrentFocusedWidget.Get())
    {
        // Walk the registry: find the first entry whose class matches
        for (const auto& Pair : ConfirmRegistry)
        {
            if (Widget->IsA(Pair.Key))
            {
                // Invoke that widget’s command
                Pair.Value(Widget);
                return true;
            }
        }

        // As a fallback, if the widget implements your own INsUINavigation, let it handle itself
        if (const INsUINavigation* Nav = GetCurrentNavInterface())
        {
            Nav->Execute_HandleUIConfirm(Widget);
            return true;
        }
    }

    return false;
}

bool UNsUISubsystem::HandleCancel()
{
    if (UWidget* Widget = CurrentFocusedWidget.Get())
    {
        // Walk the registry: find the first entry whose class matches
        for (const auto& Pair : CancelRegistry)
        {
            if (Widget->IsA(Pair.Key))
            {
                // Invoke that widget’s command
                Pair.Value(Widget);
                return true;
            }
        }

        // As a fallback, if the widget implements your own INsUINavigation, let it handle itself
        if (const INsUINavigation* Nav = GetCurrentNavInterface())
        {
            Nav->Execute_HandleUICancel(Widget);
            return true;
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

    const UUserWidget* LayerRoot = UNsUILibrary::FindRootUserWidget(InCurrentWidget);
    if (!LayerRoot)
    {
        return nullptr;
    }

    TArray<UWidget*> NavigableWidgets;
    UNsUILibrary::GatherAllNavigableDescendants(LayerRoot, NavigableWidgets);
    NavigableWidgets.Remove(InCurrentWidget);
    if (NavigableWidgets.Num() == 0)
    {
        return InCurrentWidget;
    }

    const FVector2D CurrentCenter = GetWidgetCenterInViewport(InCurrentWidget);

    FVector2D DesiredDirection(0,0);
    switch (InNavDir)
    {
        case EUINavigation::Up:    DesiredDirection = FVector2D( 0, -1); break;
        case EUINavigation::Down:  DesiredDirection = FVector2D( 0, +1); break;
        case EUINavigation::Left:  DesiredDirection = FVector2D(-1,  0); break;
        case EUINavigation::Right: DesiredDirection = FVector2D(+1,  0); break;
        default: return InCurrentWidget; // unsupported navigation
    }

    // Build half-plane filtered list
    TArray<TPair<UWidget*, FVector2D>> ValidCandidates;
    ValidCandidates.Reserve(NavigableWidgets.Num());

    for (UWidget* const Candidate : NavigableWidgets)
    {
        if (Candidate == nullptr)
        {
            continue;
        }

        const FVector2D CandidateCenter = GetWidgetCenterInViewport(Candidate);
        const FVector2D Delta      = CandidateCenter - CurrentCenter;

        // Only keep those strictly in the desired half-plane:
        if (FVector2D::DotProduct(Delta, DesiredDirection) <= 0.0f)
        {
            continue;
        }

        ValidCandidates.Emplace(Candidate, Delta);
    }

    if (ValidCandidates.Num() == 0)
    {
        // no valid moves → stay put
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

FVector2D UNsUISubsystem::GetWidgetCenterInViewport(const UWidget* const InWidget) const
{
    // Since UUserWidget::GetCachedGeometry() is protected, we have this helper inside
    // our C++ subclass so we can call it directly. It returns the center in screen coords.
    if (InWidget == nullptr)
    {
        return FVector2D::ZeroVector;
    }

    // Ask the widget for its cached geometry.  Since this is a method on UWidget (protected),
    // and UUserWidget inherits from UWidget, we can call it directly from here.
    const FGeometry Geo = InWidget->GetCachedGeometry();

    // We want the widget’s center in “absolute” (viewport) space.  We can do:
    // Geo.GetLocalToAbsoluteTransform().TransformPoint(LocalCenter) where LocalCenter = (WidgetSize / 2, WidgetSize / 2).
    const FVector2D LocalSize = Geo.GetLocalSize();
    const FVector2D LocalCenter = LocalSize * 0.5f;
    const FVector2D AbsoluteCenter = Geo.LocalToAbsolute(LocalCenter);

    return AbsoluteCenter;
}
