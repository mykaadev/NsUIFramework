#pragma once

#include "Blueprint/UserWidget.h"
#include "NsUIWidget.generated.h"

UCLASS()
class NSUIFRAMEWORK_API UNsUIWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UNsUIWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeOnInitialized() override;
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnMouseLeave(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    UFUNCTION(BlueprintNativeEvent, Category = "NsUI")
    UWidget* GetInitialFocusWidget() const;

protected:
    /** If true, moving the mouse over the widget will request keyboard focus. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NsUI")
    bool bFocusOnHover;

    void RequestFocus(EFocusCause Cause);
};
