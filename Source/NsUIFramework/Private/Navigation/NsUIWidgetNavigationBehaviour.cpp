#include "Navigation/NsUIWidgetNavigationBehaviour.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "Math/NumericLimits.h"
#include "NsUISubsystem.h"
#include "NsUIWidget.h"
#include "Widgets/SWidget.h"


void UNsUIWidgetNavigationBehaviour::OnEnter_Implementation()
{
    MoveFocusWithFallback(EUINavigation::Next);
}

void UNsUIWidgetNavigationBehaviour::OnExit_Implementation()
{
    CurrentWidgetFocus = nullptr;
    LayerStates.Empty();
}

void UNsUIWidgetNavigationBehaviour::OnNavigate_Implementation(EUINavigation Direction)
{
    MoveFocusWithFallback(Direction);
}

void UNsUIWidgetNavigationBehaviour::OnConfirm_Implementation()
{
    if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
    {
        Subsystem->ForwardConfirmToSlate();
    }
}

void UNsUIWidgetNavigationBehaviour::OnCancel_Implementation()
{
    if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
    {
        Subsystem->ForwardCancelToSlate();
    }
}

void UNsUIWidgetNavigationBehaviour::OnWidgetAdded_Implementation(UUserWidget* Screen, ENsUILayer Layer)
{
    if (!Screen)
    {
        return;
    }

    FNsUIWidgetLayerState& State = LayerStates.FindOrAdd(Layer);

    State.FocusHistory.Add(CurrentWidgetFocus);

    UWidget* FocusTarget = ResolveInitialFocus(Screen);
    if (!FocusTarget)
    {
        FocusTarget = FindFirstFocusable(Screen);
    }

    if (FocusTarget)
    {
        UpdateFocus(FocusTarget, EFocusCause::SetDirectly);
    }
}

void UNsUIWidgetNavigationBehaviour::OnWidgetRemoved_Implementation(ENsUILayer Layer, UUserWidget* Screen)
{
    if (!Screen)
    {
        return;
    }

    if (CurrentWidgetFocus.IsValid() && CurrentWidgetFocus->IsChildOf(Screen))
    {
        CurrentWidgetFocus = nullptr;
    }

    RestoreFocusForLayer(Layer);
}

void UNsUIWidgetNavigationBehaviour::OnLayerCleared_Implementation(ENsUILayer Layer)
{
    if (FNsUIWidgetLayerState* State = LayerStates.Find(Layer))
    {
        State->FocusHistory.Reset();
    }

    if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
    {
        if (CurrentWidgetFocus.IsValid())
        {
            Subsystem->UpdateCursorToWidget(nullptr);
        }
    }

    CurrentWidgetFocus = nullptr;
}

void UNsUIWidgetNavigationBehaviour::OnSlateFocusChanged_Implementation(UWidget* FocusedWidget)
{
    CurrentWidgetFocus = FocusedWidget;

    if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
    {
        if (FocusedWidget)
        {
            Subsystem->UpdateCursorToWidget(FocusedWidget);
        }
        else
        {
            Subsystem->UpdateCursorToWidget(nullptr);
        }
    }
}

void UNsUIWidgetNavigationBehaviour::OnSlateHover_Implementation(UWidget* HoveredWidget)
{
    if (!HoveredWidget || HoveredWidget == CurrentWidgetFocus.Get())
    {
        return;
    }

    if (HoveredWidget && IsWidgetFocusable(HoveredWidget) && HoveredWidget->IsVisible())
    {
        UpdateFocus(HoveredWidget, EFocusCause::Mouse);
    }
}

void UNsUIWidgetNavigationBehaviour::UpdateFocus(UWidget* Widget, EFocusCause FocusCause)
{
    if (!Widget || !IsWidgetFocusable(Widget))
    {
        return;
    }

    if (FSlateApplication::IsInitialized())
    {
        const TSharedRef<SWidget> SlateWidget = Widget->TakeWidget();

        if (const UNsUISubsystem* Subsystem = GetOwningSubsystem())
        {
            if (const ULocalPlayer* const LocalPlayer = Subsystem->GetLocalPlayer())
            {
                FSlateApplication::Get().SetUserFocus(LocalPlayer->GetControllerId(), SlateWidget, FocusCause);
            }
            else
            {
                FSlateApplication::Get().SetKeyboardFocus(SlateWidget, FocusCause);
            }
        }
    }

    CurrentWidgetFocus = Widget;

    if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
    {
        Subsystem->UpdateCursorToWidget(Widget);
    }
}

UWidget* UNsUIWidgetNavigationBehaviour::ResolveInitialFocus(UUserWidget* Screen) const
{
    if (!Screen)
    {
        return nullptr;
    }

    if (const UNsUIWidget* NsWidget = Cast<UNsUIWidget>(Screen))
    {
        if (UWidget* CustomFocus = NsWidget->GetInitialFocusWidget())
        {
            return CustomFocus;
        }
    }

    return nullptr;
}

UWidget* UNsUIWidgetNavigationBehaviour::FindFirstFocusable(UWidget* Root) const
{
    if (!Root)
    {
        return nullptr;
    }

    if (IsWidgetFocusable(Root) && Root->IsVisible())
    {
        return Root;
    }

    if (const UUserWidget* UserWidget = Cast<UUserWidget>(Root))
    {
        if (const UWidgetTree* Tree = UserWidget->WidgetTree)
        {
            TArray<UWidget*> Widgets;
            Tree->GetAllWidgets(Widgets);
            for (UWidget* Widget : Widgets)
            {
                if (Widget && IsWidgetFocusable(Widget) && Widget->IsVisible())
                {
                    return Widget;
                }
            }
        }
    }

    return nullptr;
}

void UNsUIWidgetNavigationBehaviour::RestoreFocusForLayer(ENsUILayer Layer)
{
    FNsUIWidgetLayerState* State = LayerStates.Find(Layer);
    if (!State)
    {
        return;
    }

    if (State->FocusHistory.Num() > 0)
    {
        TWeakObjectPtr<UWidget> PreviousFocus = State->FocusHistory.Pop();
        if (PreviousFocus.IsValid() && PreviousFocus->IsVisible())
        {
            UpdateFocus(PreviousFocus.Get(), EFocusCause::SetDirectly);
        }
        else
        {
            MoveFocusWithFallback(EUINavigation::Previous);
        }
    }
}

void UNsUIWidgetNavigationBehaviour::MoveFocusWithFallback(EUINavigation Direction)
{
    UNsUISubsystem* Subsystem = GetOwningSubsystem();
    if (!Subsystem)
    {
        return;
    }

    const ENsUILayer TopLayer = Subsystem->GetTopActiveLayer();
    UUserWidget* Screen = Subsystem->GetTopWidgetOnLayer(TopLayer);
    if (!Screen)
    {
        return;
    }

    UWidget* Origin = CurrentWidgetFocus.IsValid() ? CurrentWidgetFocus.Get() : FindFirstFocusable(Screen);
    if (!Origin)
    {
        return;
    }

    if (UWidget* Candidate = FindSpatialNeighbor(Screen, Origin, Direction))
    {
        UpdateFocus(Candidate, EFocusCause::Navigation);
    }
}

UWidget* UNsUIWidgetNavigationBehaviour::FindSpatialNeighbor(UUserWidget* Screen, UWidget* Origin, EUINavigation Direction) const
{
    if (!Screen || !Screen->WidgetTree || !Origin)
    {
        return nullptr;
    }

    FVector2D DirectionVector = FVector2D::ZeroVector;
    switch (Direction)
    {
    case EUINavigation::Up:
        DirectionVector = FVector2D(0.f, -1.f);
        break;
    case EUINavigation::Down:
        DirectionVector = FVector2D(0.f, 1.f);
        break;
    case EUINavigation::Left:
        DirectionVector = FVector2D(-1.f, 0.f);
        break;
    case EUINavigation::Right:
        DirectionVector = FVector2D(1.f, 0.f);
        break;
    default:
        break;
    }

    if (DirectionVector.IsNearlyZero())
    {
        return nullptr;
    }

    const UNsUISubsystem* Subsystem = GetOwningSubsystem();
    if (!Subsystem)
    {
        return nullptr;
    }

    const FVector2D OriginCenter = Subsystem->GetWidgetScreenPosition(Origin);

    TArray<UWidget*> Widgets;
    Screen->WidgetTree->GetAllWidgets(Widgets);

    UWidget* BestCandidate = nullptr;
    float BestScore = TNumericLimits<float>::Max();

    for (UWidget* Candidate : Widgets)
    {
        if (!Candidate || Candidate == Origin || !IsWidgetFocusable(Candidate) || !Candidate->IsVisible())
        {
            continue;
        }

        const FVector2D CandidateCenter = Subsystem->GetWidgetScreenPosition(Candidate);
        const FVector2D Delta = CandidateCenter - OriginCenter;

        const bool bWrongX = !FMath::IsNearlyZero(DirectionVector.X) && FMath::Sign(Delta.X) != FMath::Sign(DirectionVector.X);
        const bool bWrongY = !FMath::IsNearlyZero(DirectionVector.Y) && FMath::Sign(Delta.Y) != FMath::Sign(DirectionVector.Y);
        if (bWrongX || bWrongY)
        {
            continue;
        }

        const float Distance = Delta.SizeSquared();
        if (Distance < BestScore)
        {
            BestScore = Distance;
            BestCandidate = Candidate;
        }
    }

    return BestCandidate;
}

bool UNsUIWidgetNavigationBehaviour::IsWidgetFocusable(const UWidget* const Widget) const
{
    if (!Widget)
    {
        return false;
    }

    const TSharedPtr<SWidget> CachedWidget = Widget->GetCachedWidget();
    return CachedWidget.IsValid() && CachedWidget->SupportsKeyboardFocus();
}