#include "NsUISubsystem.h"

#include "CoreMinimal.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/NumericLimits.h"
#include "InputCoreTypes.h"
#include "NsUIDebugOverlay.h"
#include "NsUILayer.h"
#include "NsUILayout.h"
#include "NsUIWidget.h"
#include "Widgets/SWidget.h"
#include "Widgets/SObjectWidget.h"

namespace
{
constexpr int32 KeyboardUserIndex = 0;

bool IsWidgetFocusable(UWidget* Widget)
{
    if (!Widget)
    {
        return false;
    }

    const TSharedPtr<SWidget> CachedWidget = Widget->GetCachedWidget();
    return CachedWidget.IsValid() && CachedWidget->SupportsKeyboardFocus();
}
}

UNsUISubsystem::UNsUISubsystem()
    : ActiveLayout(nullptr)
    , LastInputSource(ENsUIInputSource::KeyboardGamepad)
{
    DefaultLayoutClass = UNsUILayout::StaticClass();
}

UNsUISubsystem* UNsUISubsystem::GetSubsystem()
{
    if (!GEngine)
    {
        return nullptr;
    }

    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        if (const UWorld* World = Context.World())
        {
            if (World->IsGameWorld() || World->IsPlayInEditor())
            {
                if (ULocalPlayer* LocalPlayer = World->GetFirstLocalPlayerFromController())
                {
                    if (UNsUISubsystem* Subsystem = LocalPlayer->GetSubsystem<UNsUISubsystem>())
                    {
                        return Subsystem;
                    }
                }
            }
        }
    }

    return nullptr;
}

void UNsUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (FSlateApplication::IsInitialized())
    {
        FocusChangedHandle = FSlateApplication::Get().OnFocusChanging().AddUObject(this, &UNsUISubsystem::HandleFocusChanged);
    }
}

void UNsUISubsystem::Deinitialize()
{
    if (FSlateApplication::IsInitialized())
    {
        FSlateApplication::Get().OnFocusChanging().Remove(FocusChangedHandle);
    }

    Super::Deinitialize();
}

void UNsUISubsystem::EnsureLayout()
{
    if (ActiveLayout || !GetWorld())
    {
        return;
    }

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if (!LocalPlayer)
    {
        return;
    }

    TSubclassOf<UNsUILayout> LayoutClass = DefaultLayoutClass;
    if (!LayoutClass)
    {
        LayoutClass = UNsUILayout::StaticClass();
    }
    ActiveLayout = CreateWidget<UNsUILayout>(LocalPlayer, LayoutClass);

    if (!ActiveLayout)
    {
        return;
    }

    ActiveLayout->AddToViewport();

    for (ENsUILayer Layer : {ENsUILayer::Game, ENsUILayer::Menu, ENsUILayer::Modal})
    {
        FNsUILayerState& State = LayerStates.FindOrAdd(Layer);
        State.LayerWidget = ActiveLayout->GetLayerWidget(Layer);
    }

    UpdateCursorVisibility();
}

UNsUILayer* UNsUISubsystem::GetOrCreateLayerWidget(ENsUILayer Layer)
{
    EnsureLayout();

    if (FNsUILayerState* State = LayerStates.Find(Layer))
    {
        if (State->LayerWidget.IsValid())
        {
            return State->LayerWidget.Get();
        }
    }

    if (ActiveLayout)
    {
        FNsUILayerState& State = LayerStates.FindOrAdd(Layer);
        State.LayerWidget = ActiveLayout->GetLayerWidget(Layer);
        return State.LayerWidget.Get();
    }

    return nullptr;
}

UUserWidget* UNsUISubsystem::PushWidgetToLayer(TSubclassOf<UUserWidget> ScreenClass, ENsUILayer Layer)
{
    EnsureLayout();

    if (!ScreenClass)
    {
        return nullptr;
    }

    UNsUILayer* LayerWidget = GetOrCreateLayerWidget(Layer);
    if (!LayerWidget)
    {
        return nullptr;
    }

    FNsUILayerState& State = LayerStates.FindOrAdd(Layer);
    if (CurrentFocus.IsValid() && CurrentFocus->IsChildOf(LayerWidget))
    {
        State.FocusHistory.Add(CurrentFocus);
    }
    else
    {
        State.FocusHistory.Add(nullptr);
    }

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    UUserWidget* Screen = LocalPlayer ? CreateWidget<UUserWidget>(LocalPlayer, ScreenClass) : nullptr;

    if (!Screen)
    {
        return nullptr;
    }

    LayerWidget->AddScreen(Screen);
    State.Stack.Add(Screen);

    UWidget* FocusTarget = ResolveInitialFocus(Screen);
    if (!FocusTarget)
    {
        FocusTarget = FindFirstFocusable(Screen);
    }

    if (FocusTarget)
    {
        UpdateFocus(FocusTarget, EFocusCause::SetDirectly);
    }

    UpdateCursorVisibility();

#if WITH_EDITOR
    RefreshDebugOverlay();
#endif

    return Screen;
}

UUserWidget* UNsUISubsystem::PushWidgetToLayer(TSoftClassPtr<UUserWidget> ScreenClass, ENsUILayer Layer)
{
    if (ScreenClass.IsNull())
    {
        return nullptr;
    }

    TSubclassOf<UUserWidget> LoadedClass = ScreenClass.LoadSynchronous();
    return LoadedClass ? PushWidgetToLayer(LoadedClass, Layer) : nullptr;
}

void UNsUISubsystem::PopWidgetFromLayer(ENsUILayer Layer)
{
    EnsureLayout();

    FNsUILayerState* State = LayerStates.Find(Layer);
    if (!State || State->Stack.Num() == 0)
    {
        return;
    }

    UUserWidget* RemovedWidget = State->Stack.Pop().Get();
    if (UNsUILayer* LayerWidget = State->LayerWidget.Get())
    {
        LayerWidget->RemoveScreen(RemovedWidget);
    }

    if (RemovedWidget)
    {
        RemovedWidget->RemoveFromParent();
    }

    RestoreFocusForLayer(Layer);
    UpdateCursorVisibility();

#if WITH_EDITOR
    RefreshDebugOverlay();
#endif
}

void UNsUISubsystem::PopAllWidgetsFromLayer(ENsUILayer Layer)
{
    EnsureLayout();

    FNsUILayerState* State = LayerStates.Find(Layer);
    if (!State)
    {
        return;
    }

    if (UNsUILayer* LayerWidget = State->LayerWidget.Get())
    {
        LayerWidget->ClearScreens();
    }

    State->Stack.Reset();
    State->FocusHistory.Reset();

    if (CurrentFocus.IsValid() && State->LayerWidget.IsValid() && CurrentFocus->IsChildOf(State->LayerWidget.Get()))
    {
        CurrentFocus.Reset();
        UpdateCursorToFocusedWidget();
    }

    UpdateCursorVisibility();

#if WITH_EDITOR
    RefreshDebugOverlay();
#endif
}

void UNsUISubsystem::Navigate(EUINavigation Direction)
{
    LastInputSource = ENsUIInputSource::KeyboardGamepad;
    UpdateCursorVisibility();

    FKey NavKey;
    switch (Direction)
    {
    case EUINavigation::Up:
        NavKey = EKeys::Up;
        break;
    case EUINavigation::Down:
        NavKey = EKeys::Down;
        break;
    case EUINavigation::Left:
        NavKey = EKeys::Left;
        break;
    case EUINavigation::Right:
        NavKey = EKeys::Right;
        break;
    case EUINavigation::Next:
        NavKey = EKeys::Tab;
        break;
    case EUINavigation::Previous:
        break;
    default:
        break;
    }

    TWeakPtr<SWidget> PreviousFocus = GetSlateFocus();

    if (NavKey.IsValid())
    {
        SendKeyThroughSlate(NavKey);
    }

    if (PreviousFocus != GetSlateFocus())
    {
        return;
    }

    MoveFocusWithFallback(Direction);
}

void UNsUISubsystem::Confirm()
{
    LastInputSource = ENsUIInputSource::KeyboardGamepad;
    UpdateCursorVisibility();
    SendKeyThroughSlate(EKeys::Enter);
}

void UNsUISubsystem::Cancel()
{
    LastInputSource = ENsUIInputSource::KeyboardGamepad;
    UpdateCursorVisibility();
    SendKeyThroughSlate(EKeys::Escape);
}

void UNsUISubsystem::ToggleDebugOverlay()
{
#if WITH_EDITOR
    EnsureLayout();

    if (!ActiveLayout)
    {
        return;
    }

    if (!DebugOverlay)
    {
        DebugOverlay = CreateWidget<UNsUIDebugOverlay>(GetLocalPlayer(), UNsUIDebugOverlay::StaticClass());
        if (DebugOverlay)
        {
            DebugOverlay->AddToViewport(1000);
        }
    }
    else
    {
        const ESlateVisibility NewVisibility = DebugOverlay->GetVisibility() == ESlateVisibility::Collapsed ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
        DebugOverlay->SetVisibility(NewVisibility);
    }

    RefreshDebugOverlay();
#endif
}

void UNsUISubsystem::HandleHoverFocus(UWidget* Widget)
{
    if (!Widget)
    {
        return;
    }

    if (!IsWidgetFocusable(Widget))
    {
        return;
    }

    LastInputSource = ENsUIInputSource::Mouse;
    UpdateCursorVisibility();
    UpdateFocus(Widget, EFocusCause::Mouse);
}

void UNsUISubsystem::UpdateFocus(UWidget* Widget, EFocusCause FocusCause)
{
    if (!Widget || !IsWidgetFocusable(Widget))
    {
        return;
    }

    if (FSlateApplication::IsInitialized())
    {
        if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
        {
            FSlateApplication::Get().SetUserFocus(LocalPlayer->GetControllerId(), Widget->TakeWidget(), FocusCause);
        }
        else
        {
            FSlateApplication::Get().SetKeyboardFocus(Widget->TakeWidget(), FocusCause);
        }
    }

    CurrentFocus = Widget;
    UpdateCursorToFocusedWidget();
}

UWidget* UNsUISubsystem::ResolveInitialFocus(UUserWidget* Screen) const
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

UWidget* UNsUISubsystem::FindFirstFocusable(UWidget* Root) const
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

void UNsUISubsystem::RestoreFocusForLayer(ENsUILayer Layer)
{
    FNsUILayerState* State = LayerStates.Find(Layer);
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
            return;
        }
    }

    if (State->Stack.Num() > 0)
    {
        UUserWidget* Top = State->Stack.Last().Get();
        if (UWidget* FocusTarget = ResolveInitialFocus(Top))
        {
            UpdateFocus(FocusTarget, EFocusCause::SetDirectly);
            return;
        }

        if (UWidget* FirstFocusable = FindFirstFocusable(Top))
        {
            UpdateFocus(FirstFocusable, EFocusCause::SetDirectly);
            return;
        }
    }

    CurrentFocus.Reset();
    UpdateCursorToFocusedWidget();
}

void UNsUISubsystem::SendKeyThroughSlate(const FKey& Key, const FModifierKeysState& ModifierState)
{
    if (!Key.IsValid() || !FSlateApplication::IsInitialized())
    {
        return;
    }

    const int32 UserIndex = GetLocalPlayer() ? GetLocalPlayer()->GetControllerId() : KeyboardUserIndex;
    const FKeyEvent KeyEvent(Key, ModifierState, UserIndex, false, 0, 0);

    FSlateApplication::Get().ProcessKeyDownEvent(KeyEvent);
    FSlateApplication::Get().ProcessKeyUpEvent(KeyEvent);
}

TWeakPtr<SWidget> UNsUISubsystem::GetSlateFocus() const
{
    if (!FSlateApplication::IsInitialized())
    {
        return TWeakPtr<SWidget>();
    }

    return FSlateApplication::Get().GetUserFocusedWidget(KeyboardUserIndex);
}

void UNsUISubsystem::HandleFocusChanged(const FFocusEvent& InFocusEvent, const FWeakWidgetPath& InOldPath, const TSharedPtr<SWidget>& InOldWidget, const FWidgetPath& InNewPath, const TSharedPtr<SWidget>& InNewWidget)
{
    TSharedPtr<SWidget> FocusedSlateWidget = InNewWidget;

    if (!FocusedSlateWidget.IsValid() && InNewPath.Widgets.Num() > 0)
    {
        FocusedSlateWidget = InNewPath.Widgets.Last().Widget;
    }

    if (FocusedSlateWidget.IsValid())
    {
        if (TSharedPtr<SObjectWidget> ObjectWidget = StaticCastSharedPtr<SObjectWidget>(FocusedSlateWidget))
        {
            if (UWidget* Widget = ObjectWidget->GetWidgetObject())
            {
                CurrentFocus = Widget;
                UpdateCursorToFocusedWidget();
            }
        }
    }
}

FVector2D UNsUISubsystem::GetWidgetScreenPosition(UWidget* Widget) const
{
    if (!Widget || !GetWorld())
    {
        return FVector2D::ZeroVector;
    }

    const FGeometry& Geometry = Widget->GetCachedGeometry();
    const FVector2D Absolute = Geometry.GetAbsolutePosition() + (Geometry.GetAbsoluteSize() * 0.5f);

    FVector2D ScreenPosition;
    FVector2D ViewportPosition;
    USlateBlueprintLibrary::AbsoluteToViewport(GetWorld(), Absolute, ScreenPosition, ViewportPosition);
    return ScreenPosition;
}

ENsUILayer UNsUISubsystem::GetTopActiveLayer() const
{
    const TArray<ENsUILayer> Priority = {ENsUILayer::Modal, ENsUILayer::Menu, ENsUILayer::Game};
    for (ENsUILayer Layer : Priority)
    {
        if (const FNsUILayerState* State = LayerStates.Find(Layer))
        {
            if (State->Stack.Num() > 0)
            {
                return Layer;
            }
        }
    }

    return ENsUILayer::Game;
}

UUserWidget* UNsUISubsystem::GetTopWidgetOnLayer(ENsUILayer Layer) const
{
    if (const FNsUILayerState* State = LayerStates.Find(Layer))
    {
        if (State->Stack.Num() > 0)
        {
            return State->Stack.Last().Get();
        }
    }

    return nullptr;
}

void UNsUISubsystem::MoveFocusWithFallback(EUINavigation Direction)
{
    ENsUILayer ActiveLayer = GetTopActiveLayer();

    if (CurrentFocus.IsValid())
    {
        for (const TPair<ENsUILayer, FNsUILayerState>& Pair : LayerStates)
        {
            if (Pair.Value.LayerWidget.IsValid() && CurrentFocus->IsChildOf(Pair.Value.LayerWidget.Get()))
            {
                ActiveLayer = Pair.Key;
                break;
            }
        }
    }

    UUserWidget* Screen = GetTopWidgetOnLayer(ActiveLayer);
    if (!Screen)
    {
        return;
    }

    UWidget* Origin = CurrentFocus.IsValid() ? CurrentFocus.Get() : FindFirstFocusable(Screen);
    if (!Origin)
    {
        return;
    }

    if (UWidget* Candidate = FindSpatialNeighbor(Screen, Origin, Direction))
    {
        UpdateFocus(Candidate, EFocusCause::Navigation);
    }
}

UWidget* UNsUISubsystem::FindSpatialNeighbor(UUserWidget* Screen, UWidget* Origin, EUINavigation Direction) const
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

    const FVector2D OriginCenter = GetWidgetScreenPosition(Origin);

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

        const FVector2D CandidateCenter = GetWidgetScreenPosition(Candidate);
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

void UNsUISubsystem::UpdateCursorToFocusedWidget()
{
    if (!ActiveLayout || !CurrentFocus.IsValid())
    {
        return;
    }

    const FVector2D ScreenPosition = GetWidgetScreenPosition(CurrentFocus.Get());
    ActiveLayout->UpdateCursorPosition(ScreenPosition);
}

void UNsUISubsystem::UpdateCursorVisibility()
{
    if (ActiveLayout)
    {
        ActiveLayout->SetCursorVisibility(LastInputSource == ENsUIInputSource::KeyboardGamepad);
    }
}

#if WITH_EDITOR
void UNsUISubsystem::RefreshDebugOverlay()
{
    if (!DebugOverlay || !ActiveLayout)
    {
        return;
    }

    TArray<FNsUILayerSnapshot> Snapshots;
    for (const TPair<ENsUILayer, FNsUILayerState>& Pair : LayerStates)
    {
        FNsUILayerSnapshot Snapshot;
        Snapshot.Layer = Pair.Key;

        const TArray<TWeakObjectPtr<UUserWidget>>& Stack = Pair.Value.Stack;
        for (TWeakObjectPtr<UUserWidget> WidgetPtr : Stack)
        {
            Snapshot.WidgetNames.Add(WidgetPtr.IsValid() ? WidgetPtr->GetClass()->GetName() : TEXT("Invalid"));
        }

        Snapshot.FocusedIndex = INDEX_NONE;
        for (int32 Index = Stack.Num() - 1; Index >= 0; --Index)
        {
            if (UUserWidget* Widget = Stack[Index].Get())
            {
                if (CurrentFocus.IsValid() && CurrentFocus->IsChildOf(Widget))
                {
                    Snapshot.FocusedIndex = Index;
                    break;
                }
            }
        }

        Snapshots.Add(Snapshot);
    }

    DebugOverlay->SetSnapshots(Snapshots);
}
#endif
