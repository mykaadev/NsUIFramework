#pragma once

#include "CoreMinimal.h"
#include "NsUICursorWidget.h"
#include "Templates/PimplPtr.h"
#include "NsUITweenCursorWidget.generated.h"

/** A selection cursor that glides to each new position with NsTween. Without NsTween it moves at once */
UCLASS(Abstract, Blueprintable)
class NSUIFRAMEWORKTWEEN_API UNsUITweenCursorWidget : public UNsUICursorWidget
{
    GENERATED_BODY()

// Functions
public:
    /** Constructor */
    UNsUITweenCursorWidget(const FObjectInitializer& ObjectInitializer);

    //~ Begin UUserWidget Interface
    virtual void NativeDestruct() override;
    //~ End UUserWidget Interface

protected:
    //~ Begin UNsUICursorWidget Interface
    virtual void MoveCursor(const FVector2D& ViewportPosition, bool bInstant) override;
    //~ End UNsUICursorWidget Interface

// Variables
public:
    /** The time of one glide. The default is the MotionFast token of the studio motion guide */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NsUI", meta = (ClampMin = "0", Units = "s"))
    float GlideSeconds;

private:
    /** The position that the cursor shows now */
    FVector2D ShownPosition;

    /** The glide that plays, or empty */
    TPimplPtr<struct FNsUITweenCursorGlide> Glide;
};
