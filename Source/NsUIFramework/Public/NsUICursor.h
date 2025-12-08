#pragma once

#include "Components/Widget.h"
#include "Styling/SlateBrush.h"
#include "NsUICursor.generated.h"

class SImage;

UCLASS()
class NSUIFRAMEWORK_API UNsUICursor : public UWidget
{
    GENERATED_BODY()

public:
    UNsUICursor(const FObjectInitializer& ObjectInitializer);

    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void SynchronizeProperties() override;
    virtual void ReleaseSlateResources(bool bWholeView) override;

    /** Brush used to draw the cursor. A default arrow is applied in the constructor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    FSlateBrush CursorBrush;

    void SetCursorBrush(const FSlateBrush& InBrush);

private:
    TSharedPtr<SImage> MyImage;
};
