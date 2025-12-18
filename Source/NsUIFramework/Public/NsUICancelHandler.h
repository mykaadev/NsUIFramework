#pragma once

#include "UObject/Interface.h"
#include "NsUICancelHandler.generated.h"

UINTERFACE(BlueprintType)
class NSUIFRAMEWORK_API UNsUICancelHandler : public UInterface
{
    GENERATED_BODY()
};

class NSUIFRAMEWORK_API INsUICancelHandler
{
    GENERATED_BODY()

public:
    /** Return true if the widget handled cancel (no further unwind/pop will happen). */
    UFUNCTION(BlueprintNativeEvent, Category = "NsUI|Navigation")
    bool HandleNsUICancel();
};
