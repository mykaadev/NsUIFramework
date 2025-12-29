#include "Navigation/NsUIWidgetNavigationBehaviour.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "Math/NumericLimits.h"
#include "NsUIInputHandler.h"
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
    SlateToWidgetMap.Empty();
}

void UNsUIWidgetNavigationBehaviour::OnTick_Implementation(float DeltaTime)
{
    SyncHoverFocusWithSlate();
}

void UNsUIWidgetNavigationBehaviour::OnNavigate_Implementation(EUINavigation Direction)
{
    if (CurrentWidgetFocus.IsValid() &&
        CurrentWidgetFocus->GetClass()->ImplementsInterface(UNsUIInputHandler::StaticClass()))
    {
        const bool bHandled = INsUIInputHandler::Execute_HandleNsUINavigate(CurrentWidgetFocus.Get(), Direction);
        if (bHandled)
        {
            return;
        }
    }

    MoveFocusWithFallback(Direction);
}

void UNsUIWidgetNavigationBehaviour::OnConfirm_Implementation()
{
    if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
    {
        if (CurrentWidgetFocus.IsValid() &&
            CurrentWidgetFocus->GetClass()->ImplementsInterface(UNsUIInputHandler::StaticClass()))
        {
            const bool bHandled = INsUIInputHandler::Execute_HandleNsUIConfirm(CurrentWidgetFocus.Get());
            if (bHandled)
            {
                return;
            }
        }

        Subsystem->ForwardConfirmToSlate();
    }
}

void UNsUIWidgetNavigationBehaviour::OnCancel_Implementation()
{
    if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
    {
        if (CurrentWidgetFocus.IsValid() &&
            CurrentWidgetFocus->GetClass()->ImplementsInterface(UNsUIInputHandler::StaticClass()))
        {
            const bool bHandled = INsUIInputHandler::Execute_HandleNsUICancel(CurrentWidgetFocus.Get());
            if (bHandled)
            {
                return;
            }
        }

        const ENsUILayer ActiveLayer = Subsystem->GetTopActiveLayer();
        if (UUserWidget* TopWidget = Subsystem->GetTopWidgetOnLayer(ActiveLayer))
        {
            Subsystem->PopWidgetFromLayer(ActiveLayer);
            return;
        }

        Subsystem->ForwardCancelToSlate();
    }
}

void UNsUIWidgetNavigationBehaviour::OnWidgetAdded_Implementation(UUserWidget* Screen, ENsUILayer Layer)
{
    if (!Screen)
    {
        return;
    }

    RegisterWidgetTree(Screen);

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
    else
    {
        CurrentWidgetFocus = nullptr;

        if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
        {
            Subsystem->UpdateCursorToWidget(nullptr);
        }
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

    EFocusCause RequestedFocusCause = FocusCause;

    if (RequestedFocusCause == EFocusCause::SetDirectly)
    {
        RequestedFocusCause = EFocusCause::Navigation;
    }

    const bool bIsMouseFocus = RequestedFocusCause == EFocusCause::Mouse;
    const bool bIsNavigationInput = RequestedFocusCause == EFocusCause::Navigation;

    if (FSlateApplication::IsInitialized())
    {
        const TSharedRef<SWidget> SlateWidget = Widget->TakeWidget();

        if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
        {
            if (const ULocalPlayer* const LocalPlayer = Subsystem->GetLocalPlayer())
            {
                FSlateApplication::Get().SetUserFocus(LocalPlayer->GetControllerId(), SlateWidget, RequestedFocusCause);

                // Also assign keyboard focus so the widget is visually focused immediately.
                FSlateApplication::Get().SetKeyboardFocus(SlateWidget, RequestedFocusCause);
            }
            else
            {
                FSlateApplication::Get().SetKeyboardFocus(SlateWidget, RequestedFocusCause);
            }
        }
    }

    CurrentWidgetFocus = Widget;

    if (UNsUISubsystem* Subsystem = GetOwningSubsystem())
    {
        Subsystem->UpdateCursorToWidget(Widget);
    }
}

void UNsUIWidgetNavigationBehaviour::RegisterWidgetTree(UUserWidget* Root)
{
    if (!Root)
    {
        return;
    }

    TArray<UWidget*> Widgets;
    Root->WidgetTree->GetAllWidgets(Widgets);

    for (UWidget* Widget : Widgets)
    {
        if (!Widget)
        {
            continue;
        }

        // This will cache the Slate widget if it hasn’t been created yet
        TSharedRef<SWidget> SlateWidget = Widget->TakeWidget();
        SlateToWidgetMap.Add(SlateWidget, Widget);
    }
}

UWidget* UNsUIWidgetNavigationBehaviour::ResolveWidgetFromSlate(const FWidgetPath& NewPath, const TSharedPtr<SWidget>& NewWidget) const
{
    // Try the direct widget from OnFocusChanging
    if (NewWidget.IsValid())
    {
        if (const TWeakObjectPtr<UWidget>* Found = SlateToWidgetMap.Find(NewWidget))
        {
            return Found->Get();
        }
    }

    // Fallback: climb the path from leaf to root and see
    //    if any of those Slate widgets are in our map.
    for (int32 Index = NewPath.Widgets.Num() - 1; Index >= 0; --Index)
    {
        const TSharedRef<SWidget>& Candidate = NewPath.Widgets[Index].Widget;

        if (const TWeakObjectPtr<UWidget>* Found = SlateToWidgetMap.Find(Candidate))
        {
            return Found->Get();
        }
    }

    return nullptr;
}

UWidget* UNsUIWidgetNavigationBehaviour::FindWidgetUnderPointer(const FVector2D& ScreenSpacePosition) const
{
    if (!FSlateApplication::IsInitialized())
    {
        return nullptr;
    }

    FSlateApplication& SlateApp = FSlateApplication::Get();
    const FWidgetPath HoveredPath = SlateApp.LocateWindowUnderMouse(ScreenSpacePosition, SlateApp.GetInteractiveTopLevelWindows());
    if (!HoveredPath.IsValid())
    {
        return nullptr;
    }

    const TSharedPtr<SWidget> HoveredSlateWidget = HoveredPath.Widgets.Last().Widget;
    return ResolveWidgetFromSlate(HoveredPath, HoveredSlateWidget);
}

void UNsUIWidgetNavigationBehaviour::SyncHoverFocusWithSlate()
{
    if (!FSlateApplication::IsInitialized())
    {
        return;
    }

    FSlateApplication& SlateApp = FSlateApplication::Get();
    const FWidgetPath HoveredPath = SlateApp.LocateWindowUnderMouse(SlateApp.GetCursorPos(), SlateApp.GetInteractiveTopLevelWindows());

    if (!HoveredPath.IsValid())
    {
        return;
    }

    for (int32 Index = HoveredPath.Widgets.Num() - 1; Index >= 0; --Index)
    {
        const TSharedRef<SWidget>& HoveredSlateWidget = HoveredPath.Widgets[Index].Widget;

        if (const TWeakObjectPtr<UWidget>* Found = SlateToWidgetMap.Find(HoveredSlateWidget))
        {
            OnSlateHover(Found->Get());
            break;
        }
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
