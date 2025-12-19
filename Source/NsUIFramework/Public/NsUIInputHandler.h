#pragma once

#include "UObject/Interface.h"
#include "InputCoreTypes.h"
#include "NsUIInputHandler.generated.h"

UINTERFACE(BlueprintType)
class NSUIFRAMEWORK_API UNsUIInputHandler : public UInterface
{
    GENERATED_BODY()
};

/**
 * Optional interface for widgets that want to handle NsUI navigation/confirm/cancel directly.
 * Return true from a handler to consume the input and stop further processing.
 */
class NSUIFRAMEWORK_API INsUIInputHandler
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, Category = "NsUI|Navigation")
    bool HandleNsUIConfirm();

    UFUNCTION(BlueprintNativeEvent, Category = "NsUI|Navigation")
    bool HandleNsUICancel();

    UFUNCTION(BlueprintNativeEvent, Category = "NsUI|Navigation")
    bool HandleNsUINavigate(EUINavigation Direction);
};
