#pragma once

#include "Components/Widget.h"
#include "NsUIFrameworkTypes.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Templates/SharedPointer.h"
#include "Tickable.h"
#include "UObject/SoftObjectPtr.h"
#include "NsUISubsystem.generated.h"

class UNsUILayout;
class UNsUILayer;
class UNsUICursor;
class UNsUIWidget;
class UUserWidget;
class UWidget;
class SWidget;
class UNsUINavigationBehaviour;

/**
 * Central entry point for the retro UI stack. Manages layout creation, widget stacks per
 * layer, focus management (widgets/actors/locations), retro cursor syncing, and optional
 * pluggable navigation behaviours that can drive bespoke interactions.
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUISubsystem : public ULocalPlayerSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:
    UNsUISubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    static UNsUISubsystem* GetSubsystem();

    /** Push a widget class onto the given UI layer (synchronous class). */
    UUserWidget* PushWidgetToLayer(TSubclassOf<UUserWidget> ScreenClass, ENsUILayer Layer);
    /** Push a soft widget class onto the given UI layer (loads if needed). */
    UUserWidget* PushWidgetToLayer(TSoftClassPtr<UUserWidget> ScreenClass, ENsUILayer Layer);
    /** Remove the top-most widget from a layer stack. */
    void PopWidgetFromLayer(ENsUILayer Layer);
    /** Clear an entire layer stack. */
    void PopAllWidgetsFromLayer(ENsUILayer Layer);

    /** Route directional navigation into Slate, with fallback spatial search. */
    void Navigate(EUINavigation Direction);
    /** Trigger confirm/accept on the current focus or behavior. */
    void Confirm();
    /** Trigger cancel/back on the current behavior or focused widget. */
    void Cancel();
    /** Helper for navigation behaviours to forward confirm to Slate when no custom behaviour is needed. */
    void ForwardConfirmToSlate();
    /** Helper for navigation behaviours to forward cancel to Slate when no custom behaviour is needed. */
    void ForwardCancelToSlate();

    /** Returns true when NsUI is currently controlling focus/navigation (layout or selection session active). */
    bool IsFrameworkActive() const;

    void ToggleDebugOverlay();

    /** Convenience getter for widget focus (returns null for actor/location focus kinds). */
    UWidget* GetCurrentFocusedWidget() const { return CurrentFocus.Widget.Get(); }
    /** Update focus state when Slate hover changes. */
    void HandleHoverFocus(UWidget* Widget);

    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }

    /** Swap in a new navigation behaviour, calling lifecycle events appropriately. */
    void SetNavigationBehaviour(UNsUINavigationBehaviour* NewBehaviour);
    /** Begin a selection session over provided actors/locations using a behaviour. */
    void BeginSelectionSession(UNsUINavigationBehaviour* Behaviour, const TArray<AActor*>& Actors, const TArray<FVector>& Locations);
    /** Tear down any active selection session and clear focus. */
    void EndSelectionSession();
    /** Cycle to the next focus target in the active selection session. */
    void FocusNextTarget();
    /** Cycle to the previous focus target in the active selection session. */
    void FocusPreviousTarget();
    /** Notify behaviors and cursor when focus changes. */
    void NotifyFocusChanged();
    /** Navigate widgets using NsUI's spatial logic (for default UI navigation). */
    void NavigateWidgets(EUINavigation Direction);
    /** Navigate selection targets using default focus stepping. */
    void NavigateSelectionTargets(EUINavigation Direction);
    /** Navigate selection targets filtered by kind (actors/locations/widgets). */
    void NavigateTargetsByKind(EUINavigation Direction, ENsUIFocusKind AllowedKind);
    /** Directly focus a target by index inside the current selection list. */
    void FocusTargetAtIndex(int32 Index);
    /** Read-only accessors for navigation behaviours. */
    const TArray<FNsUIFocusTarget>& GetFocusTargets() const { return FocusTargets; }
    int32 GetFocusIndex() const { return FocusIndex; }

private:
    /** Internal representation of a single layer's widget stack and focus history. */
    struct FNsUILayerState
    {
        TWeakObjectPtr<UNsUILayer> LayerWidget;
        TArray<TWeakObjectPtr<UUserWidget>> Stack;
        TArray<TWeakObjectPtr<UWidget>> FocusHistory;
    };

    UPROPERTY()
    /** Currently instantiated layout widget (owns the cursor and per-layer containers). */
    TObjectPtr<UNsUILayout> ActiveLayout;

    UPROPERTY(EditDefaultsOnly, Category = "NsUI")
    /** Layout class used when none is explicitly provided by the game. */
    TSubclassOf<UNsUILayout> DefaultLayoutClass;

#if WITH_EDITORONLY_DATA
    UPROPERTY()
    /** Editor-only overlay showing focus and stack debugging information. */
    TObjectPtr<class UNsUIDebugOverlay> DebugOverlay;
#endif

    /** Map of layer enum to stack state. */
    TMap<ENsUILayer, FNsUILayerState> LayerStates;
    /** Tracks most recent input device to toggle cursor visibility rules. */
    ENsUIInputSource LastInputSource;
    /** Current focus target driving the retro cursor. */
    FNsUIFocusTarget CurrentFocus;
    /** Active selection session targets (actors/locations). */
    TArray<FNsUIFocusTarget> FocusTargets;
    /** Index into FocusTargets for cycling. */
    int32 FocusIndex = INDEX_NONE;
    /** Handles for Slate focus/tick delegates. */
    FDelegateHandle FocusChangedHandle;
    FDelegateHandle SlatePreTickHandle;

    /** Reverse lookup from Slate widget to UWidget for hover/focus reconciliation. */
    TMap<TWeakPtr<SWidget>, TWeakObjectPtr<UWidget>> SlateToWidgetMap;

    UPROPERTY()
    /** Currently bound navigation behaviour that receives confirm/hover/cancel/tick events. */
    TObjectPtr<UNsUINavigationBehaviour> ActiveNavigationBehaviour;

    void EnsureLayout();
    UNsUILayer* GetOrCreateLayerWidget(ENsUILayer Layer);
    void UpdateFocus(UWidget* Widget, EFocusCause FocusCause);
    UWidget* ResolveInitialFocus(UUserWidget* Screen) const;
    UWidget* FindFirstFocusable(UWidget* Root) const;
    void RestoreFocusForLayer(ENsUILayer Layer);

    void SendKeyThroughSlate(const FKey& Key, const FModifierKeysState& ModifierState = FModifierKeysState());
    TWeakPtr<SWidget> GetSlateFocus() const;
    UWidget* ResolveWidgetFromSlate(const FWidgetPath& NewPath, const TSharedPtr<SWidget>& NewWidget) const;

    void HandleFocusChanged(const FFocusEvent& InFocusEvent, const FWeakWidgetPath& InOldPath, const TSharedPtr<SWidget>& InOldWidget, const FWidgetPath& InNewPath, const TSharedPtr<SWidget>& InNewWidget);

    void RegisterWidgetTree(UUserWidget* Root);

    FVector2D GetWidgetScreenPosition(UWidget* Widget) const;
    FVector2D GetActorScreenPosition(AActor* Actor) const;
    FVector2D GetLocationScreenPosition(const FVector& WorldLocation) const;
    ENsUILayer GetTopActiveLayer() const;
    UUserWidget* GetTopWidgetOnLayer(ENsUILayer Layer) const;
    void MoveFocusWithFallback(EUINavigation Direction);
    UWidget* FindSpatialNeighbor(UUserWidget* Screen, UWidget* Origin, EUINavigation Direction) const;

    void UpdateCursorToCurrentFocus();
    void UpdateCursorVisibility();
    void SyncHoverFocusWithSlate(float InDeltaTime);

#if WITH_EDITOR
    void RefreshDebugOverlay();
#endif
};
