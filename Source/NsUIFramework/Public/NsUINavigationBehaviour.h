#pragma once

#include "CoreMinimal.h"
#include "NsUIFrameworkTypes.h"
#include "NsUINavigationBehaviour.generated.h"

class UNsUISubsystem;

/**
 * Swappable navigation/interaction policy that consumes confirm/hover/cancel input and optional
 * ticking during selection sessions. Implement in Blueprint or C++ to drive custom behavior when
 * navigating actors, widgets, or arbitrary world locations.
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
    void OnConfirm(const FNsUIFocusTarget& Target);

    /** Called whenever focus changes to a new target. */
    UFUNCTION(BlueprintNativeEvent)
    void OnHover(const FNsUIFocusTarget& Target);

    /** Called when cancel/back input is pressed. */
    UFUNCTION(BlueprintNativeEvent)
    void OnCancel();

private:
    /** Owning subsystem (weak to avoid lifetime issues). */
    UPROPERTY()
    TWeakObjectPtr<UNsUISubsystem> OwningSubsystem;
};
