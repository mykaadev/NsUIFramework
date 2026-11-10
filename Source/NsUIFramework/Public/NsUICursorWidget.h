#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NsUICursorWidget.generated.h"

/**
 * The selection cursor. It shows next to the selection while the player uses keys or a gamepad
 * Make a Blueprint child with the art, and name it in the NsUI settings. The cursor never takes clicks
 */
UCLASS(Abstract, Blueprintable)
class NSUIFRAMEWORK_API UNsUICursorWidget : public UUserWidget
{
    GENERATED_BODY()

// Functions
public:
    /** Constructor */
    UNsUICursorWidget(const FObjectInitializer& ObjectInitializer);

    //~ Begin UUserWidget Interface
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    //~ End UUserWidget Interface

    /** Follows a focused Slate widget */
    void FollowSlateWidget(const TSharedPtr<class SWidget>& Target);

    /** Follows the selection of a selection mode */
    void FollowSelectionMode(class UNsUISelectionMode* Mode);

    /** Hides the cursor and stops following */
    void HideCursor();

    /** Called when the cursor gets a new position. Instant is true when the cursor was hidden before. Return true when the Blueprint moves the cursor itself */
    UFUNCTION(BlueprintNativeEvent, Category = "NsUI")
    bool OnCursorMoved(FVector2D ViewportPosition, bool bInstant);

protected:
    /** Moves the cursor to a viewport position. The default moves it at once. A child class can animate the move */
    virtual void MoveCursor(const FVector2D& ViewportPosition, bool bInstant);

private:
    /** Gives the viewport position of the target, or false when there is none */
    bool GetTargetPosition(FVector2D& OutViewportPosition);

// Variables
public:
    /** The point of the target that the cursor points at. (0, 0) is the top left of the target, (1, 1) the bottom right */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NsUI")
    FVector2D AnchorInTarget;

    /** The point of the cursor that sits on the anchor. (0, 0) is the top left of the cursor, (1, 1) the bottom right */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NsUI")
    FVector2D CursorAlignment;

private:
    /** The focused Slate widget to follow */
    TWeakPtr<class SWidget> TargetWidget;

    /** The selection mode to follow */
    UPROPERTY(Transient)
    TWeakObjectPtr<class UNsUISelectionMode> TargetMode;

    /** The last position given to MoveCursor */
    FVector2D LastPosition;

    /** True while the cursor shows */
    bool bShowing;
};
