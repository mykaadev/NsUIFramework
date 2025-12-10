#pragma once

#include "CoreMinimal.h"
#include "NsUIFrameworkTypes.h"
#include "NsUISelectionBehavior.generated.h"

UCLASS(Abstract, Blueprintable)
class NSUIFRAMEWORK_API UNsUISelectionBehavior : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent)
    void OnEnter();

    UFUNCTION(BlueprintNativeEvent)
    void OnExit();

    UFUNCTION(BlueprintNativeEvent)
    void OnTick(float DeltaTime);

    UFUNCTION(BlueprintNativeEvent)
    void OnConfirm(const FNsUIFocusTarget& Target);

    UFUNCTION(BlueprintNativeEvent)
    void OnHover(const FNsUIFocusTarget& Target);

    UFUNCTION(BlueprintNativeEvent)
    void OnCancel();
};
