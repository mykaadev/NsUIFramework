#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NsUITestWidget.generated.h"

/** An empty widget for the automation tests. UUserWidget is abstract, so the tests need a class that CreateWidget can make */
UCLASS(NotBlueprintable, HideDropdown, Transient)
class UNsUITestWidget : public UUserWidget
{
    GENERATED_BODY()
};
