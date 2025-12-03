#pragma once

#include "NsUIFrameworkTypes.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Templates/SharedPointer.h"
#include "UObject/SoftObjectPtr.h"
#include "NsUISubsystem.generated.h"

class UNsUILayout;
class UNsUILayer;
class UNsUICursor;
class UNsUIWidget;
class UUserWidget;
class UWidget;
class SWidget;

UCLASS()
class NSUIFRAMEWORK_API UNsUISubsystem : public ULocalPlayerSubsystem
{
    GENERATED_BODY()

public:
    UNsUISubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    static UNsUISubsystem* GetSubsystem();

    UUserWidget* PushWidgetToLayer(TSubclassOf<UUserWidget> ScreenClass, ENsUILayer Layer);
    UUserWidget* PushWidgetToLayer(TSoftClassPtr<UUserWidget> ScreenClass, ENsUILayer Layer);
    void PopWidgetFromLayer(ENsUILayer Layer);
    void PopAllWidgetsFromLayer(ENsUILayer Layer);

    void Navigate(EUINavigation Direction);
    void Confirm();
    void Cancel();

    void ToggleDebugOverlay();

    UWidget* GetCurrentFocusedWidget() const { return CurrentFocus.Get(); }
    void HandleHoverFocus(UWidget* Widget);

private:
    struct FNsUILayerState
    {
        TWeakObjectPtr<UNsUILayer> LayerWidget;
        TArray<TWeakObjectPtr<UUserWidget>> Stack;
        TArray<TWeakObjectPtr<UWidget>> FocusHistory;
    };

    UPROPERTY()
    TObjectPtr<UNsUILayout> ActiveLayout;

    UPROPERTY(EditDefaultsOnly, Category = "NsUI")
    TSubclassOf<UNsUILayout> DefaultLayoutClass;

#if WITH_EDITOR
    UPROPERTY()
    TObjectPtr<class UNsUIDebugOverlay> DebugOverlay;
#endif

    TMap<ENsUILayer, FNsUILayerState> LayerStates;
    ENsUIInputSource LastInputSource;
    TWeakObjectPtr<UWidget> CurrentFocus;
    FDelegateHandle FocusChangedHandle;

    void EnsureLayout();
    UNsUILayer* GetOrCreateLayerWidget(ENsUILayer Layer);
    void UpdateFocus(UWidget* Widget, EFocusCause FocusCause);
    UWidget* ResolveInitialFocus(UUserWidget* Screen) const;
    UWidget* FindFirstFocusable(UWidget* Root) const;
    void RestoreFocusForLayer(ENsUILayer Layer);

    void SendKeyThroughSlate(const FKey& Key, const FModifierKeysState& ModifierState = FModifierKeysState());
    TWeakPtr<SWidget> GetSlateFocus() const;
    void HandleFocusChanged(const FFocusEvent& InFocusEvent, const FWeakWidgetPath& InOldPath, const TSharedPtr<SWidget>& InOldWidget, const FWidgetPath& InNewPath, const TSharedPtr<SWidget>& InNewWidget);

    FVector2D GetWidgetScreenPosition(UWidget* Widget) const;
    ENsUILayer GetTopActiveLayer() const;
    UUserWidget* GetTopWidgetOnLayer(ENsUILayer Layer) const;
    void MoveFocusWithFallback(EUINavigation Direction);
    UWidget* FindSpatialNeighbor(UUserWidget* Screen, UWidget* Origin, EUINavigation Direction) const;

    void UpdateCursorToFocusedWidget();
    void UpdateCursorVisibility();

#if WITH_EDITOR
    void RefreshDebugOverlay();
#endif
};
