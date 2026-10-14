#pragma once
#include "Blueprint/UserWidget.h"
#include "NsUICursor.generated.h"

class SImage;

/** Displays the selection target without receiving input. */
UCLASS()
class NSUIFRAMEWORK_API UNsUICursor : public UUserWidget
{
    GENERATED_BODY()
public:
    UNsUICursor(const FObjectInitializer& Initializer);
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;
    void SetCursorBrush(const FSlateBrush& Brush);
    void Present(const TOptional<FVector2D>& Position, float DeltaTime);
private:
    UPROPERTY()
    FSlateBrush CursorBrush;
    TSharedPtr<SImage> Image;
    FVector2D CurrentPosition = FVector2D::ZeroVector;
    bool bHasPosition = false;
};
