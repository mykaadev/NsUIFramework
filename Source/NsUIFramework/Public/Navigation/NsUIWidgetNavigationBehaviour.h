#pragma once

#include "NsUINavigationBehaviour.h"
#include "NsUICancelHandler.h"
#include "Widgets/SWidget.h"
#include "NsUIWidgetNavigationBehaviour.generated.h"

struct FNsUIWidgetLayerState
{
    TArray<TWeakObjectPtr<UWidget>> FocusHistory;
};

struct FWidgetPath;

/** Navigation behaviour that owns UMG focus movement and cursor updates. */
UCLASS(Blueprintable)
class NSUIFRAMEWORK_API UNsUIWidgetNavigationBehaviour : public UNsUINavigationBehaviour
{
    GENERATED_BODY()

public:
    virtual void OnEnter_Implementation() override;
    virtual void OnExit_Implementation() override;
    virtual void OnTick_Implementation(float DeltaTime) override;
    virtual void OnNavigate_Implementation(EUINavigation Direction) override;
    virtual void OnConfirm_Implementation() override;
    virtual void OnCancel_Implementation() override;
    virtual void OnWidgetAdded_Implementation(UUserWidget* Screen, ENsUILayer Layer) override;
    virtual void OnWidgetRemoved_Implementation(ENsUILayer Layer, UUserWidget* Screen) override;
    virtual void OnLayerCleared_Implementation(ENsUILayer Layer) override;
    virtual void OnSlateFocusChanged_Implementation(UWidget* FocusedWidget) override;
    virtual void OnSlateHover_Implementation(UWidget* HoveredWidget) override;

    /** Register a widget tree so Slate widgets can be resolved back to UWidgets. */
    void RegisterWidgetTree(UUserWidget* Root);
    /** Resolve a Slate widget/path back to a UWidget within registered trees. */
    UWidget* ResolveWidgetFromSlate(const FWidgetPath& NewPath, const TSharedPtr<SWidget>& NewWidget) const;
    /** Locate a UWidget under the provided screen position if it belongs to NsUI. */
    UWidget* FindWidgetUnderPointer(const FVector2D& ScreenSpacePosition) const;
    /** Sync hover focus against Slate when using mouse input. */
    void SyncHoverFocusWithSlate();

private:

    void UpdateFocus(UWidget* Widget, EFocusCause FocusCause);
    UWidget* ResolveInitialFocus(UUserWidget* Screen) const;
    UWidget* FindFirstFocusable(UWidget* Root) const;
    void RestoreFocusForLayer(ENsUILayer Layer);
    void MoveFocusWithFallback(EUINavigation Direction);
    UWidget* FindSpatialNeighbor(UUserWidget* Screen, UWidget* Origin, EUINavigation Direction) const;

    bool IsWidgetFocusable(const UWidget* const Widget) const;

    UPROPERTY()
    TWeakObjectPtr<UWidget> CurrentWidgetFocus;

    TMap<ENsUILayer, FNsUIWidgetLayerState> LayerStates;
    TMap<TWeakPtr<SWidget>, TWeakObjectPtr<UWidget>> SlateToWidgetMap;
};
