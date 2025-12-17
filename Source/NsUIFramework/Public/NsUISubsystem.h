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
 * layer, widget focus management, retro cursor syncing, and pluggable navigation behaviours
 * that own their own navigation state (UMG, actor selection, etc.).
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

    /** Route directional navigation into the active behaviour. */
    void Navigate(EUINavigation Direction);

    /** Trigger confirm/accept on the current focus or behavior. */
    void Confirm();

    /** Trigger cancel/back on the current behavior or focused widget. */
    void Cancel();

    /** Helper for navigation behaviours to forward confirm to Slate when no custom behaviour is needed. */
    void ForwardConfirmToSlate();

    /** Helper for navigation behaviours to forward cancel to Slate when no custom behaviour is needed. */
    void ForwardCancelToSlate();

    /** Returns true when NsUI is currently controlling focus/navigation (layout present or a navigation behaviour is active). */
    bool IsFrameworkActive() const;

    void ToggleDebugOverlay();

    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }

    /** Swap in a new navigation behaviour, calling lifecycle events appropriately. */
    void SetNavigationBehaviour(UNsUINavigationBehaviour* NewBehaviour);

    /** Update the retro cursor to follow a widget. */
    void UpdateCursorToWidget(UWidget* Widget);
    /** Update the retro cursor to follow an actor. */
    void UpdateCursorToActor(AActor* Actor);
    /** Update the retro cursor to follow an arbitrary location. */
    void UpdateCursorToLocation(const FVector& WorldLocation);

    /** Project a widget into viewport space. */
    FVector2D GetWidgetScreenPosition(UWidget* Widget) const;

    /** Project an actor into viewport space. */
    FVector2D GetActorScreenPosition(AActor* Actor) const;

    /** Project a world-space location into viewport space. */
    FVector2D GetLocationScreenPosition(const FVector& WorldLocation) const;

    /** Retrieve the top-most active layer (Modal > Menu > Game). */
    ENsUILayer GetTopActiveLayer() const;

    /** Handle Hover Focus */
    void HandleHoverFocus(UWidget* Widget);

    /** Retrieve the top-most widget on a given layer. */
    UUserWidget* GetTopWidgetOnLayer(ENsUILayer Layer) const;

    /** Is forwarding input */
    bool IsForwardingSyntheticInputToSlate() const { return bForwardingSyntheticInputToSlate; }

	/** True when a navigation behaviour is bound (meaning NsUI intends to own confirm/cancel/navigation). */
	bool HasActiveNavigationBehaviour() const;

private:
    /** Internal representation of a single layer's widget stack and focus history. */
    struct FNsUILayerState
    {
        TWeakObjectPtr<UNsUILayer> LayerWidget;
        TArray<TWeakObjectPtr<UUserWidget>> Stack;
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
    /** Handles for Slate focus/tick delegates. */
    FDelegateHandle FocusChangedHandle;
    FDelegateHandle SlatePreTickHandle;

    /** Reverse lookup from Slate widget to UWidget for hover/focus reconciliation. */
    TMap<TWeakPtr<SWidget>, TWeakObjectPtr<UWidget>> SlateToWidgetMap;

    /** Currently bound navigation behaviour that receives confirm/hover/cancel/tick events. */
    UPROPERTY()
    TObjectPtr<UNsUINavigationBehaviour> ActiveNavigationBehaviour;

    /** Currently forwarding synthetic input */
    UPROPERTY()
    bool bForwardingSyntheticInputToSlate;

    void EnsureLayout();
    UNsUILayer* GetOrCreateLayerWidget(ENsUILayer Layer);

    void SendKeyThroughSlate(const FKey& Key, const FModifierKeysState& ModifierState = FModifierKeysState());
    TWeakPtr<SWidget> GetSlateFocus() const;
    UWidget* ResolveWidgetFromSlate(const FWidgetPath& NewPath, const TSharedPtr<SWidget>& NewWidget) const;

    void HandleFocusChanged(const FFocusEvent& InFocusEvent, const FWeakWidgetPath& InOldPath, const TSharedPtr<SWidget>& InOldWidget, const FWidgetPath& InNewPath, const TSharedPtr<SWidget>& InNewWidget);

    void RegisterWidgetTree(UUserWidget* Root);
    void UpdateCursorVisibility();
    void SyncHoverFocusWithSlate(float InDeltaTime);

#if WITH_EDITOR
    void RefreshDebugOverlay();
#endif
};
