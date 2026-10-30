#pragma once

#include "Blueprint/UserWidget.h"
#include "NsUIWorldPopupWidget.generated.h"

class UTextBlock;

/**
 * One floating text over a world point, for example "+3 coins". The world popup subsystem places it each frame,
 * moves it up, and fades it out. The Blueprint child holds the layout and the motion values
 */
UCLASS(Abstract, Blueprintable)
class NSUIFRAMEWORK_API UNsUIWorldPopupWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Writes the text and the color into the popup and resets its motion. The subsystem calls it when the popup starts */
    void StartPopup(const FText& Text, const FLinearColor& Color);

    /** Applies the opacity and the scale for an age in seconds. Returns false when the popup is over */
    bool ApplyAge(float Age);

    /** The age in seconds at which the popup ends */
    float GetLifetime() const { return Lifetime; }

    /** The upward speed in cm/s */
    float GetRiseSpeed() const { return RiseSpeed; }

    /** The greatest distance from the camera in cm at which the popup shows */
    float GetMaxDistance() const { return MaxDistance; }

    /** True when the popup hides while a wall is between the camera and the popup */
    bool HidesWhenBlocked() const { return bHideWhenBlocked; }

    /** The most popups of this class that show at the same time for one player */
    int32 GetMaxActive() const { return MaxActive; }

    /** The viewport order of the popup. A lower value draws under the other viewport widgets */
    int32 GetViewportZOrder() const { return ViewportZOrder; }

    /** The height in cm between popups that follow the same actor at the same time */
    float GetStackHeight() const { return StackHeight; }

protected:
    /** Lets the Blueprint react to a new popup, for example to show an icon */
    UFUNCTION(BlueprintImplementableEvent, Category="NsUI|World Popup")
    void OnPopupStarted(const FText& Text, FLinearColor Color);

    /** Shows the text of the popup. Optional in the layout */
    UPROPERTY(BlueprintReadOnly, Category="NsUI|World Popup", meta=(BindWidgetOptional))
    TObjectPtr<UTextBlock> PopupText;

    /** The time in seconds that the popup shows */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NsUI|World Popup", meta=(ClampMin="0.1", Units="s"))
    float Lifetime = 1.2f;

    /** The upward speed of the popup in the world */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NsUI|World Popup", meta=(ClampMin="0", Units="cm/s"))
    float RiseSpeed = 45.f;

    /** The popup hides beyond this distance from the camera */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NsUI|World Popup", meta=(ClampMin="0", Units="cm"))
    float MaxDistance = 3000.f;

    /** The time in seconds of the fade in at the start */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NsUI|World Popup", meta=(ClampMin="0", Units="s"))
    float FadeInSeconds = 0.1f;

    /** The time in seconds of the fade out at the end */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NsUI|World Popup", meta=(ClampMin="0", Units="s"))
    float FadeOutSeconds = 0.4f;

    /** The scale at the start. The popup shrinks to scale 1 in the pop time */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NsUI|World Popup", meta=(ClampMin="0"))
    float PopScale = 1.f;

    /** The time in seconds from the pop scale to scale 1 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NsUI|World Popup", meta=(ClampMin="0", Units="s"))
    float PopSeconds = 0.2f;

    /** True hides the popup while a wall blocks the view from the camera */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NsUI|World Popup")
    bool bHideWhenBlocked = true;

    /** The most popups of this class at the same time for one player. A new popup over the limit ends the oldest one */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NsUI|World Popup", meta=(ClampMin="1"))
    int32 MaxActive = 4;

    /** The extra height of a popup over each popup that follows the same actor now, so the texts do not overlap */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NsUI|World Popup", meta=(ClampMin="0", Units="cm"))
    float StackHeight = 30.f;

    /** The viewport order of the popup. A lower value draws under the other viewport widgets */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NsUI|World Popup")
    int32 ViewportZOrder = -100;
};
