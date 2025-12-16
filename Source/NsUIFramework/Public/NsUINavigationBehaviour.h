#pragma once

#include "CoreMinimal.h"
#include "NsUIFrameworkTypes.h"
#include "NsUINavigationBehaviour.generated.h"

class UNsUISubsystem;
class UUserWidget;
class UWidget;

/**
 * Swappable navigation/interaction policy that consumes confirm/cancel input and optional ticking.
 * Implement in Blueprint or C++ to drive custom behavior when navigating widgets, actors, or other
 * targets without relying on a centralized focus target struct.
 */
UCLASS(Abstract, Blueprintable)
class NSUIFRAMEWORK_API UNsUINavigationBehaviour : public UObject
{
    GENERATED_BODY()

public:
    /** Owning subsystem set when the behaviour is activated. */
    UFUNCTION(BlueprintPure, Category = "NsUI|Navigation")
    UNsUISubsystem* GetOwningSubsystem() const { return OwningSubsystem.Get(); }

    /** Internal hook used by the subsystem to provide context for navigation decisions. */
    void SetOwningSubsystem(UNsUISubsystem* InSubsystem) { OwningSubsystem = InSubsystem; }

    /** Called when this behavior becomes active. */
    UFUNCTION(BlueprintNativeEvent)
    void OnEnter();

    /** Called when the behavior is removed or replaced. */
    UFUNCTION(BlueprintNativeEvent)
    void OnExit();

    /** Per-frame callback for driving previews, timers, etc. */
    UFUNCTION(BlueprintNativeEvent)
    void OnTick(float DeltaTime);

    /** Called for directional navigation input (up/down/left/right/next/previous). */
    UFUNCTION(BlueprintNativeEvent)
    void OnNavigate(EUINavigation Direction);

    /** Called when confirm/accept input is pressed. */
    UFUNCTION(BlueprintNativeEvent)
    void OnConfirm();

    /** Called when cancel/back input is pressed. */
    UFUNCTION(BlueprintNativeEvent)
    void OnCancel();

    /** Notified when a widget is pushed onto a layer. Default implementation is a no-op. */
    UFUNCTION(BlueprintNativeEvent)
    void OnWidgetAdded(UUserWidget* Screen, ENsUILayer Layer);

    /** Notified when a widget is removed from a layer. Default implementation is a no-op. */
    UFUNCTION(BlueprintNativeEvent)
    void OnWidgetRemoved(ENsUILayer Layer, UUserWidget* Screen);

    /** Notified when a full layer has been cleared. Default implementation is a no-op. */
    UFUNCTION(BlueprintNativeEvent)
    void OnLayerCleared(ENsUILayer Layer);

    /** Notified when Slate focus changes to a specific widget. Default implementation is a no-op. */
    UFUNCTION(BlueprintNativeEvent)
    void OnSlateFocusChanged(UWidget* FocusedWidget);

    /** Notified when Slate hover highlights a widget. Default implementation is a no-op. */
    UFUNCTION(BlueprintNativeEvent)
    void OnSlateHover(UWidget* HoveredWidget);

private:
    /** Owning subsystem (weak to avoid lifetime issues). */
    UPROPERTY()
    TWeakObjectPtr<UNsUISubsystem> OwningSubsystem;
};
