#pragma once

#include "CoreMinimal.h"
#include "NsUIFrameworkTypes.h"
#include "NsUISelectionBehavior.generated.h"

/**
 * Swappable interaction policy that consumes confirm/hover/cancel input and optional ticking
 * during selection sessions. Implement in Blueprint or C++ to drive custom behavior when
 * navigating actors, widgets, or arbitrary world locations.
 */
UCLASS(Abstract, Blueprintable)
class NSUIFRAMEWORK_API UNsUISelectionBehavior : public UObject
{
    GENERATED_BODY()

public:
    /** Called when this behavior becomes active. */
    UFUNCTION(BlueprintNativeEvent)
    void OnEnter();

    /** Called when the behavior is removed or replaced. */
    UFUNCTION(BlueprintNativeEvent)
    void OnExit();

    /** Per-frame callback for driving previews, timers, etc. */
    UFUNCTION(BlueprintNativeEvent)
    void OnTick(float DeltaTime);

    /** Called when confirm/accept input is pressed. */
    UFUNCTION(BlueprintNativeEvent)
    void OnConfirm(const FNsUIFocusTarget& Target);

    /** Called whenever focus changes to a new target. */
    UFUNCTION(BlueprintNativeEvent)
    void OnHover(const FNsUIFocusTarget& Target);

    /** Called when cancel/back input is pressed. */
    UFUNCTION(BlueprintNativeEvent)
    void OnCancel();
};
