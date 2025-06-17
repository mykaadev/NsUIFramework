#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NsUICursor.generated.h"

/**
 * Simple widget used to visually indicate which widget currently has keyboard focus.
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUICursor : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Move the cursor so that it overlays the target widget. */
    UFUNCTION(BlueprintCallable, Category="Cursor")
    void MoveToWidget(UWidget* InTarget);
};

