#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NsUIWorldPopupWidget.generated.h"

/**
 * One floating text over a world point, for example "+3 coins". Make a Blueprint child: it holds the look, the motion values, and the optional Appear and Disappear animations
 * The world popup subsystem moves it each frame, because it follows a world point
 */
UCLASS(Abstract, Blueprintable)
class NSUIFRAMEWORK_API UNsUIWorldPopupWidget : public UUserWidget
{
    GENERATED_BODY()

// Functions
public:
    /** Constructor */
    UNsUIWorldPopupWidget(const FObjectInitializer& ObjectInitializer);

    /** Starts the popup with a text and a color, and plays the Appear animation */
    void StartPopup(const FText& Text, const FLinearColor& Color);

    /** Plays the Disappear animation */
    void PlayDisappear();

    /** Returns the length of the Disappear animation, or 0 */
    float GetDisappearSeconds() const;

    /** Called when the popup starts. Use it for more than the text, for example to show an icon */
    UFUNCTION(BlueprintImplementableEvent, Category = "NsUI")
    void OnPopupStarted(const FText& Text, FLinearColor Color);

// Variables
public:
    /** The time that the popup shows */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NsUI|Popup", meta = (ClampMin = "0.01", Units = "s"))
    float Lifetime;

    /** The upward speed in the world */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NsUI|Popup", meta = (Units = "cm/s"))
    float RiseSpeed;

    /** The popup hides beyond this distance from the camera */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NsUI|Popup", meta = (ClampMin = "0", Units = "cm"))
    float MaxDistance;

    /** True hides the popup while a Visibility trace from the camera hits something */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NsUI|Popup")
    bool bHideWhenBlocked;

    /** The most popups of this class for one player. A new popup over the limit ends the oldest one */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NsUI|Popup", meta = (ClampMin = "1"))
    int32 MaxActive;

    /** The extra height for each popup that already follows the same actor */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NsUI|Popup", meta = (Units = "cm"))
    float StackHeight;

    /** The viewport order. A lower value draws under other viewport widgets */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NsUI|Popup")
    int32 ViewportZOrder;

    /** The point of the popup that sits on the world point. (0.5, 1) is the bottom center */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NsUI|Popup")
    FVector2D ScreenAlignment;

protected:
    /** The optional text of the popup */
    UPROPERTY(BlueprintReadOnly, Category = "NsUI|Popup", meta = (BindWidgetOptional))
    TObjectPtr<class UTextBlock> PopupText;

    /** The optional animation that plays when the popup starts */
    UPROPERTY(Transient, BlueprintReadOnly, Category = "NsUI|Popup", meta = (BindWidgetAnimOptional))
    TObjectPtr<class UWidgetAnimation> Appear;

    /** The optional animation that plays at the end of the lifetime */
    UPROPERTY(Transient, BlueprintReadOnly, Category = "NsUI|Popup", meta = (BindWidgetAnimOptional))
    TObjectPtr<class UWidgetAnimation> Disappear;
};
