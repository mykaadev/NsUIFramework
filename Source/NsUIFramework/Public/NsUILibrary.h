#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "NsUIFrameworkTypes.h"
#include "UObject/SoftObjectPtr.h"

#include "Blueprint/UserWidget.h"
#include "NsUILibrary.generated.h"

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
};
