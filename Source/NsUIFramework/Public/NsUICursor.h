#pragma once

#include "Components/Widget.h"
#include "Styling/SlateBrush.h"
#include "NsUICursor.generated.h"

class SImage;
class UWidget;
class AActor;
class UNsUILayout;

UCLASS()
class NSUIFRAMEWORK_API UNsUICursor : public UWidget
{
    GENERATED_BODY()

public:
    UNsUICursor(const FObjectInitializer& ObjectInitializer);

    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void SynchronizeProperties() override;
    virtual void ReleaseSlateResources(bool bWholeView) override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    /** Brush used to draw the cursor. A default arrow is applied in the constructor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    FSlateBrush CursorBrush;

    void SetCursorBrush(const FSlateBrush& InBrush);

    /** Configure the layout that owns this cursor. */
    void SetOwningLayout(UNsUILayout* InLayout);

    /** Drive the cursor toward a widget target. */
    void SetTargetWidget(UWidget* Widget);
    /** Drive the cursor toward an actor target. */
    void SetTargetActor(AActor* Actor);
    /** Drive the cursor toward an arbitrary world location. */
    void SetTargetLocation(const FVector& WorldLocation);

    /** Returns true when the cursor has a valid target to chase. */
    bool HasTarget() const;

private:
    enum class ENsUICursorTargetType
    {
        None,
        Widget,
        Actor,
        Location
    };

    FVector2D ResolveWidgetScreenPosition(UWidget* Widget) const;
    FVector2D ResolveActorScreenPosition(AActor* Actor) const;
    FVector2D ResolveLocationScreenPosition(const FVector& WorldLocation) const;
    FVector2D ResolveDesiredScreenPosition() const;
    bool HasValidTarget() const;
    void SetCursorVisible(bool bVisible);

    TSharedPtr<SImage> MyImage;

    UPROPERTY()
    TObjectPtr<UNsUILayout> OwningLayout;

    /** Cached position the cursor is moving toward. */
    FVector2D CursorTargetPosition;
    /** Current cursor position for interpolation. */
    FVector2D CurrentCursorPosition;
    /** Current cursor target type. */
    ENsUICursorTargetType CursorTargetType = ENsUICursorTargetType::None;
    /** Widget driving the cursor position. */
    TWeakObjectPtr<UWidget> CursorTargetWidget;
    /** Actor driving the cursor position. */
    TWeakObjectPtr<AActor> CursorTargetActor;
    /** World-space point driving the cursor position. */
    FVector CursorTargetWorldLocation;
    /** True once the cursor has been positioned at least once. */
    bool bCursorHasPosition = false;
};
