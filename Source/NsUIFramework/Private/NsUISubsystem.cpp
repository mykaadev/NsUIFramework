#include "CoreMinimal.h"
#include "NsUISubsystem.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/NavigationConfig.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Math/NumericLimits.h"
#include "Navigation/NsUIWidgetNavigationBehaviour.h"
#include "NsUIDebugOverlay.h"
#include "NsUILayer.h"
#include "NsUILayout.h"
#include "NsUINavigationBehaviour.h"
#include "Stats/Stats.h"
#include "Templates/GuardValue.h"
#include "Widgets/SWidget.h"

namespace
{
    TSharedPtr<FNavigationConfig> GOriginalNavigationConfig;
    TSharedPtr<IInputProcessor> GNavigationInputPreProcessor;

    /** Custom input pre-processor that forwards mouse clicks to NsUI confirm/cancel when active. */
    class FNsUINavigationInputPreProcessor : public IInputProcessor
    {
    public:
        explicit FNsUINavigationInputPreProcessor(TWeakObjectPtr<UNsUISubsystem> InSubsystem)
            : Subsystem(InSubsystem)
        {
        }

        virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override
        {
            // No-op (we only care about mouse button events)
        }

        virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
        {
            UNsUISubsystem* Sys = Subsystem.Get();
            if (!Sys || !Sys->IsFrameworkActive() || Sys->IsForwardingSyntheticInputToSlate())
            {
                return false;
            }

            const FKey Button = MouseEvent.GetEffectingButton();
            if (Button == EKeys::LeftMouseButton)
            {
                if (UNsUINavigationBehaviour* Behaviour = Sys->GetActiveNavigationBehaviour())
                {
                    if (Behaviour->OnMouseButtonDown(MouseEvent.GetScreenSpacePosition(), Button))
                    {
                        return true;
                    }
                }

                if (UNsUIWidgetNavigationBehaviour* WidgetBehaviour = Cast<UNsUIWidgetNavigationBehaviour>(Sys->GetActiveNavigationBehaviour()))
                {
                    if (UWidget* TargetWidget = WidgetBehaviour->FindWidgetUnderPointer(MouseEvent.GetScreenSpacePosition()))
                    {
                        Sys->HandleHoverFocus(TargetWidget);
                    }
                }

                // Let Slate and any other input handlers process the click normally.
                return false;
            }

            return false;
        }

    private:
        TWeakObjectPtr<UNsUISubsystem> Subsystem;
    };

    /** Custom navigation config that can route nav events into UNsUISubsystem when active. */
    class FNsUINavigationConfig : public FNavigationConfig
    {
        public:

        FNsUINavigationConfig(TWeakObjectPtr<UNsUISubsystem> InSubsystem, const TSharedRef<FNavigationConfig>& InWrappedConfig)
            : Subsystem(InSubsystem)
            , WrappedConfig(InWrappedConfig)
        {
            // Start with the same settings as the wrapped config (key mappings, flags, etc.)
            *static_cast<FNavigationConfig*>(this) = *InWrappedConfig;
        }

        virtual EUINavigation GetNavigationDirectionFromKey(const FKeyEvent& InKeyEvent) const override
        {
            const EUINavigation Direction = WrappedConfig->GetNavigationDirectionFromKey(InKeyEvent);

            UNsUISubsystem* Sys = Subsystem.Get();
            if (!Sys || !Sys->IsFrameworkActive() || Sys->IsForwardingSyntheticInputToSlate())
            {
                // Framework inactive -> behave exactly like the original config.
                return Direction;
            }

            if (Direction != EUINavigation::Invalid)
            {
                Sys->Navigate(Direction);
                // Returning Invalid here tells Slate "we handled the nav; don't do built-in focus move".
                return EUINavigation::Invalid;
            }

            return EUINavigation::Invalid;
        }

        // NOTE: Navigation actions (Accept/Back) are forwarded into the subsystem as well so Confirm/Cancel
        // follow the same routing rules as directional navigation when NsUI is active.

        virtual EUINavigationAction GetNavigationActionFromKey(const FKeyEvent& InKeyEvent) const override
        {
            const EUINavigationAction Action = WrappedConfig->GetNavigationActionFromKey(InKeyEvent);

            UNsUISubsystem* Sys = Subsystem.Get();
            if (!Sys || !Sys->IsFrameworkActive() || Sys->IsForwardingSyntheticInputToSlate())
            {
                return Action;
            }

            switch (Action)
            {
            case EUINavigationAction::Accept:
                Sys->Confirm();
                return EUINavigationAction::Invalid;

            case EUINavigationAction::Back:
                Sys->Cancel();
                return EUINavigationAction::Invalid;

            default:
                break;
            }

            return Action;
        }

    private:
        TWeakObjectPtr<UNsUISubsystem> Subsystem;
        TSharedRef<FNavigationConfig> WrappedConfig;
    };

    // Slate treats user index 0 as the default keyboard/gamepad user; keep constant local for clarity.
    constexpr int32 KeyboardUserIndex = 0;

}

UNsUISubsystem::UNsUISubsystem()
    : ActiveLayout(nullptr)
    , LastInputSource(ENsUIInputSource::KeyboardGamepad)
    , ActiveNavigationBehaviour(nullptr)
    , bForwardingSyntheticInputToSlate(false)
{
    DefaultLayoutClass = UNsUILayout::StaticClass();
}

UNsUISubsystem* UNsUISubsystem::GetSubsystem()
{
    if (!GEngine)
    {
        return nullptr;
    }

    // Prefer the actual game viewport the user is looking at
    if (const UGameViewportClient* Viewport = GEngine->GameViewport)
    {
        if (const UGameInstance* GameInstance = Viewport->GetGameInstance())
        {
            if (const ULocalPlayer* LocalPlayer = GameInstance->GetFirstGamePlayer())
            {
                return LocalPlayer->GetSubsystem<UNsUISubsystem>();
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
        SlatePreTickHandle = FSlateApplication::Get().OnPreTick().AddUObject(this, &UNsUISubsystem::SyncHoverFocusWithSlate);
        GNavigationInputPreProcessor = MakeShared<FNsUINavigationInputPreProcessor>(this);
        FSlateApplication::Get().RegisterInputPreProcessor(GNavigationInputPreProcessor);

        if (!GOriginalNavigationConfig.IsValid())
        {
            GOriginalNavigationConfig = FSlateApplication::Get().GetNavigationConfig();

            const TSharedRef<FNavigationConfig> WrappedRef = GOriginalNavigationConfig.ToSharedRef();
            const TSharedRef<FNavigationConfig> NsConfig = MakeShared<FNsUINavigationConfig>(this, WrappedRef);

            FSlateApplication::Get().SetNavigationConfig(NsConfig);
        }
    }
}

void UNsUISubsystem::Deinitialize()
{
    if (FSlateApplication::IsInitialized() && FocusChangedHandle.IsValid())
    {
        FSlateApplication::Get().OnFocusChanging().Remove(FocusChangedHandle);
        FocusChangedHandle.Reset();
    }

    if (FSlateApplication::IsInitialized() && SlatePreTickHandle.IsValid())
    {
        FSlateApplication::Get().OnPreTick().Remove(SlatePreTickHandle);
        SlatePreTickHandle.Reset();
    }

    if (FSlateApplication::IsInitialized() && GNavigationInputPreProcessor.IsValid())
    {
        FSlateApplication::Get().UnregisterInputPreProcessor(GNavigationInputPreProcessor);
        GNavigationInputPreProcessor.Reset();
    }

    if (FSlateApplication::IsInitialized() && GOriginalNavigationConfig.IsValid())
    {
        FSlateApplication::Get().SetNavigationConfig(GOriginalNavigationConfig.ToSharedRef());
        GOriginalNavigationConfig.Reset();
    }

    Super::Deinitialize();
}

void UNsUISubsystem::EnsureLayout()
{
    const UWorld* const World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    const ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if (LocalPlayer == nullptr)
    {
        return;
    }

    // If the existing layout is for another world or is otherwise invalid, destroy it
    if (ActiveLayout)
    {
        const bool bWrongWorld = ActiveLayout->GetWorld() != World;

        const bool bSlateInvalid = !ActiveLayout->GetCachedWidget().IsValid();

        if (bWrongWorld || bSlateInvalid)
        {
            ActiveLayout->RemoveFromParent();
            ActiveLayout = nullptr;
            LayerStates.Empty();
            SlateToWidgetMap.Empty();
        }
    }

    // (Re)create layout if needed
    if (!ActiveLayout)
    {
        TSubclassOf<UNsUILayout> LayoutClass = DefaultLayoutClass;
        if (!LayoutClass)
        {
            LayoutClass = UNsUILayout::StaticClass();
        }

        ActiveLayout = CreateWidget<UNsUILayout>(LocalPlayer->GetPlayerController(World), LayoutClass);

        if (ActiveLayout == nullptr)
        {
            return;
        }

        ActiveLayout->AddToViewport();

        LayerStates.Empty();
        for (ENsUILayer Layer : { ENsUILayer::Game, ENsUILayer::Menu, ENsUILayer::Modal })
        {
            FNsUILayerState& State = LayerStates.FindOrAdd(Layer);
            State.LayerWidget = ActiveLayout->GetLayerWidget(Layer);
        }

        if (!ActiveNavigationBehaviour)
        {
            SetNavigationBehaviour(NewObject<UNsUIWidgetNavigationBehaviour>(this));
        }

        UpdateCursorVisibility();
        return;
    }

    // Existing layout: make sure it is hooked into viewport and layers are bound
    if (!ActiveLayout->IsInViewport())
    {
        ActiveLayout->AddToViewport();
    }

    if (LayerStates.Num() == 0)
    {
        for (ENsUILayer Layer : { ENsUILayer::Game, ENsUILayer::Menu, ENsUILayer::Modal })
        {
            FNsUILayerState& State = LayerStates.FindOrAdd(Layer);
            State.LayerWidget = ActiveLayout->GetLayerWidget(Layer);
        }
    }
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

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    UUserWidget* Screen = LocalPlayer ? CreateWidget<UUserWidget>(LocalPlayer->GetPlayerController(LocalPlayer->GetWorld()), ScreenClass) : nullptr;

    if (!Screen)
    {
        return nullptr;
    }

    LayerWidget->AddScreen(Screen);
    LayerStates.FindOrAdd(Layer).Stack.Add(Screen);
    if (UNsUIWidgetNavigationBehaviour* WidgetBehaviour = Cast<UNsUIWidgetNavigationBehaviour>(ActiveNavigationBehaviour))
    {
        WidgetBehaviour->RegisterWidgetTree(Screen);
    }

    if (ActiveNavigationBehaviour)
    {
        ActiveNavigationBehaviour->OnWidgetAdded(Screen, Layer);
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

    if (ActiveNavigationBehaviour)
    {
        ActiveNavigationBehaviour->OnWidgetRemoved(Layer, RemovedWidget);
    }
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

    if (ActiveNavigationBehaviour)
    {
        ActiveNavigationBehaviour->OnLayerCleared(Layer);
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

    if (ActiveNavigationBehaviour)
    {
        ActiveNavigationBehaviour->OnNavigate(Direction);
    }
}

void UNsUISubsystem::Confirm()
{
    LastInputSource = ENsUIInputSource::KeyboardGamepad;
    UpdateCursorVisibility();

    if (ActiveNavigationBehaviour)
    {
        ActiveNavigationBehaviour->OnConfirm();
        return;
    }

    SendKeyThroughSlate(EKeys::Enter);
}

void UNsUISubsystem::Cancel()
{
    LastInputSource = ENsUIInputSource::KeyboardGamepad;
    UpdateCursorVisibility();

    if (ActiveNavigationBehaviour)
    {
        ActiveNavigationBehaviour->OnCancel();
        return;
    }

    SendKeyThroughSlate(EKeys::Escape);
}

void UNsUISubsystem::ForwardConfirmToSlate()
{
    if (bForwardingSyntheticInputToSlate)
    {
        return;
    }

    SendKeyThroughSlate(EKeys::Enter);
}

void UNsUISubsystem::ForwardCancelToSlate()
{
    if (bForwardingSyntheticInputToSlate)
    {
        return;
    }

    SendKeyThroughSlate(EKeys::Escape);
}

void UNsUISubsystem::Tick(float DeltaTime)
{
    if (ActiveNavigationBehaviour)
    {
        ActiveNavigationBehaviour->OnTick(DeltaTime);
    }
}

bool UNsUISubsystem::IsFrameworkActive() const
{
    if (ActiveNavigationBehaviour != nullptr)
    {
        return true;
    }

    if (!ActiveLayout)
    {
        return false;
    }

    for (const TPair<ENsUILayer, FNsUILayerState>& Pair : LayerStates)
    {
        const TArray<TWeakObjectPtr<UUserWidget>>& Stack = Pair.Value.Stack;
        if (Stack.Num() > 0)
        {
            return true;
        }
    }

    return false;
}

TStatId UNsUISubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UNsUISubsystem, STATGROUP_Tickables);
}

void UNsUISubsystem::SetNavigationBehaviour(UNsUINavigationBehaviour* NewBehaviour)
{
    if (ActiveNavigationBehaviour == NewBehaviour)
    {
        return;
    }

    if (ActiveNavigationBehaviour)
    {
        ActiveNavigationBehaviour->OnExit();
        ActiveNavigationBehaviour->SetOwningSubsystem(nullptr);
    }

    ActiveNavigationBehaviour = NewBehaviour;

    if (ActiveNavigationBehaviour)
    {
        ActiveNavigationBehaviour->SetOwningSubsystem(this);
        ActiveNavigationBehaviour->OnEnter();
    }
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
        DebugOverlay = CreateWidget<UNsUIDebugOverlay>(GetLocalPlayer()->GetPlayerController(GetLocalPlayer()->GetWorld()), UNsUIDebugOverlay::StaticClass());
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
    if (ActiveNavigationBehaviour)
    {
        LastInputSource = ENsUIInputSource::Mouse;
        UpdateCursorVisibility();
        ActiveNavigationBehaviour->OnSlateHover(Widget);
    }
}

void UNsUISubsystem::SendKeyThroughSlate(const FKey& Key, const FModifierKeysState& ModifierState)
{
    if (!Key.IsValid() || !FSlateApplication::IsInitialized())
    {
        return;
    }

    // Prevent nested re-entry from stomping the flag back to false mid-call
    if (bForwardingSyntheticInputToSlate)
    {
        return;
    }

    TGuardValue<bool> ForwardingGuard(bForwardingSyntheticInputToSlate, true);
    const int32 UserIndex = GetLocalPlayer() ? GetLocalPlayer()->GetControllerId() : KeyboardUserIndex;
    const FKeyEvent KeyEvent(Key, ModifierState, UserIndex, false, 0, 0);

    FSlateApplication::Get().ProcessKeyDownEvent(KeyEvent);
    FSlateApplication::Get().ProcessKeyUpEvent(KeyEvent);
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
    if (InFocusEvent.GetCause() == EFocusCause::Mouse)
    {
        LastInputSource = ENsUIInputSource::Mouse;
        UpdateCursorVisibility();
    }

    if (UNsUIWidgetNavigationBehaviour* WidgetBehaviour = Cast<UNsUIWidgetNavigationBehaviour>(ActiveNavigationBehaviour))
    {
        UWidget* NewFocusedWidget = WidgetBehaviour->ResolveWidgetFromSlate(InNewPath, InNewWidget);
        WidgetBehaviour->OnSlateFocusChanged(IsValid(NewFocusedWidget) ? NewFocusedWidget : nullptr);
    }
}

void UNsUISubsystem::SyncHoverFocusWithSlate(float InDeltaTime)
{
    if (LastInputSource != ENsUIInputSource::Mouse)
    {
        return;
    }

    if (UNsUIWidgetNavigationBehaviour* WidgetBehaviour = Cast<UNsUIWidgetNavigationBehaviour>(ActiveNavigationBehaviour))
    {
        WidgetBehaviour->SyncHoverFocusWithSlate();
    }
}

FVector2D UNsUISubsystem::GetWidgetScreenPosition(UWidget* Widget) const
{
    if (Widget == nullptr || GetWorld() == nullptr || ActiveLayout == nullptr)
    {
        return FVector2D::ZeroVector;
    }

    const FGeometry& Geometry = Widget->GetCachedGeometry();

    // Pick a point inside the widget using the anchor
    const FVector2D Absolute = Geometry.GetAbsolutePosition() + (Geometry.GetAbsoluteSize() * ActiveLayout->GetCursorWidgetSnapAnchor());

    FVector2D PixelPosition;
    FVector2D InOutViewportPosition;
    USlateBlueprintLibrary::AbsoluteToViewport(GetWorld(), Absolute, PixelPosition, InOutViewportPosition);
    return InOutViewportPosition;
}

FVector2D UNsUISubsystem::GetActorScreenPosition(AActor* Actor) const
{
    if (!Actor || !GetWorld())
    {
        return FVector2D::ZeroVector;
    }

    ULocalPlayer* LP = GetLocalPlayer();
    APlayerController* PC = LP ? LP->GetPlayerController(GetWorld()) : nullptr;
    if (!PC)
    {
        return FVector2D::ZeroVector;
    }

    const FBox Bounds = Actor->GetComponentsBoundingBox(true);
    const FVector WorldCenter = Bounds.IsValid ? Bounds.GetCenter() : Actor->GetActorLocation();

    FVector2D Screen;
    if (UGameplayStatics::ProjectWorldToScreen(PC, WorldCenter, Screen))
    {
        return Screen;
    }

    return FVector2D::ZeroVector;
}

FVector2D UNsUISubsystem::GetLocationScreenPosition(const FVector& WorldLocation) const
{
    if (!GetWorld())
    {
        return FVector2D::ZeroVector;
    }

    ULocalPlayer* LP = GetLocalPlayer();
    APlayerController* PC = LP ? LP->GetPlayerController(GetWorld()) : nullptr;
    if (!PC)
    {
        return FVector2D::ZeroVector;
    }

    FVector2D Screen;
    if (UGameplayStatics::ProjectWorldToScreen(PC, WorldLocation, Screen))
    {
        return Screen;
    }

    return FVector2D::ZeroVector;
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

void UNsUISubsystem::UpdateCursorToWidget(UWidget* Widget)
{
    if (!ActiveLayout || !Widget)
    {
        return;
    }

    const FVector2D ScreenPosition = GetWidgetScreenPosition(Widget);
    ActiveLayout->UpdateCursorPosition(ScreenPosition);
}

void UNsUISubsystem::UpdateCursorToActor(AActor* Actor)
{
    if (!ActiveLayout || !Actor)
    {
        return;
    }

    const FVector2D ScreenPosition = GetActorScreenPosition(Actor);
    ActiveLayout->UpdateCursorPosition(ScreenPosition);
}

void UNsUISubsystem::UpdateCursorToLocation(const FVector& WorldLocation)
{
    if (!ActiveLayout)
    {
        return;
    }

    const FVector2D ScreenPosition = GetLocationScreenPosition(WorldLocation);
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

        Snapshots.Add(Snapshot);
    }

    DebugOverlay->SetSnapshots(Snapshots);
}
#endif
