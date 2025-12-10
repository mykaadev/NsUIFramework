#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NsUIFrameworkTypes.h"
#include "NsUISelectionBehavior.h"
#include "UObject/SoftObjectPtr.h"

#include "Blueprint/UserWidget.h"
#include "NsUILibrary.generated.h"

class AActor;

UCLASS()
class NSUIFRAMEWORK_API UNsUILibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "NsUI")
    static UUserWidget* Push(TSoftClassPtr<UUserWidget> ScreenClass, ENsUILayer Layer);

    static UUserWidget* Push(TSubclassOf<UUserWidget> ScreenClass, ENsUILayer Layer);

    UFUNCTION(BlueprintCallable, Category = "NsUI")
    static void Pop(ENsUILayer Layer);

    UFUNCTION(BlueprintCallable, Category = "NsUI")
    static void PopAll(ENsUILayer Layer);

    UFUNCTION(BlueprintCallable, Category = "NsUI")
    static void ToggleDebugOverlay();

    UFUNCTION(BlueprintCallable, Category = "NsUI|Selection")
    static void SetSelectionBehavior(UNsUISelectionBehavior* Behavior);

    UFUNCTION(BlueprintCallable, Category = "NsUI|Selection")
    static void BeginSelectionSession(UNsUISelectionBehavior* Behavior, const TArray<AActor*>& Actors, const TArray<FVector>& Locations);

    UFUNCTION(BlueprintCallable, Category = "NsUI|Selection")
    static void EndSelectionSession();

    UFUNCTION(BlueprintCallable, Category = "NsUI|Selection")
    static void FocusNextTarget();

    UFUNCTION(BlueprintCallable, Category = "NsUI|Selection")
    static void FocusPreviousTarget();
};
