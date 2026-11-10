#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NsUIWidgetInterface.h"
#include "NsUIWidget.generated.h"

/** The parent class for widgets that NsUIFramework opens. A call to RemoveFromParent closes the widget through the framework */
UCLASS(Abstract)
class NSUIFRAMEWORK_API UNsUIWidget : public UUserWidget, public INsUIWidgetInterface
{
    GENERATED_BODY()

// Functions
public:
    //~ Begin UWidget Interface
    virtual void RemoveFromParent() override;
    //~ End UWidget Interface
};
