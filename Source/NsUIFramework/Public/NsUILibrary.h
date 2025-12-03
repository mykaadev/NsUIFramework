#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NsUIFrameworkTypes.h"
#include "NsUILibrary.generated.h"

UCLASS()
class NSUIFRAMEWORK_API UNsUILibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "NsUI", meta = (WorldContext = "WorldContextObject"))
    static UUserWidget* Push(UObject* WorldContextObject, TSubclassOf<UUserWidget> ScreenClass, ENsUILayer Layer);

    UFUNCTION(BlueprintCallable, Category = "NsUI", meta = (WorldContext = "WorldContextObject"))
    static void Pop(UObject* WorldContextObject, ENsUILayer Layer);

    UFUNCTION(BlueprintCallable, Category = "NsUI", meta = (WorldContext = "WorldContextObject"))
    static void PopAll(UObject* WorldContextObject, ENsUILayer Layer);

    UFUNCTION(BlueprintCallable, Category = "NsUI", meta = (WorldContext = "WorldContextObject"))
    static void ToggleDebugOverlay(UObject* WorldContextObject);
};
