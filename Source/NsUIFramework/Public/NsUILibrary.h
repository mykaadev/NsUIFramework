#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NsUIFrameworkTypes.h"
#include "NsUINavigationBehaviour.h"
#include "UObject/SoftObjectPtr.h"

#include "Blueprint/UserWidget.h"
#include "NsUILibrary.generated.h"

class AActor;

/** Blueprint surface for driving the NsUI retro UI subsystem. */
UCLASS()
class NSUIFRAMEWORK_API UNsUILibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Push a screen onto a layer using a soft reference (loads on demand). */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    static UUserWidget* Push(TSoftClassPtr<UUserWidget> ScreenClass, ENsUILayer Layer);

    /** Push a screen onto a layer using a hard reference. */
    static UUserWidget* Push(TSubclassOf<UUserWidget> ScreenClass, ENsUILayer Layer);

    /** Pop the top-most screen from the specified layer. */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    static void Pop(ENsUILayer Layer);

    /** Remove all screens from a layer. */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    static void PopAll(ENsUILayer Layer);

    /** Toggle the in-editor debug overlay that visualizes focus and stacks. */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    static void ToggleDebugOverlay();

    /** Swap in a navigation behaviour for confirm/hover/cancel routing. */
    UFUNCTION(BlueprintCallable, Category = "NsUI|Selection")
    static void SetNavigationBehaviour(UNsUINavigationBehaviour* Behaviour);

    /** Begin a selection session targeting actors and/or world locations. */
    UFUNCTION(BlueprintCallable, Category = "NsUI|Selection")
    static void BeginSelectionSession(UNsUINavigationBehaviour* Behaviour, const TArray<AActor*>& Actors, const TArray<FVector>& Locations);

    /** End the active selection session and clear focus/behavior. */
    UFUNCTION(BlueprintCallable, Category = "NsUI|Selection")
    static void EndSelectionSession();

    /** Step forward through selection targets. */
    UFUNCTION(BlueprintCallable, Category = "NsUI|Selection")
    static void FocusNextTarget();

    /** Step backward through selection targets. */
    UFUNCTION(BlueprintCallable, Category = "NsUI|Selection")
    static void FocusPreviousTarget();
};
