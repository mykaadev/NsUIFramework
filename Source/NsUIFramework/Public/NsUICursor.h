// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "NsUICursor.generated.h"


class UCanvasPanel;
class UCanvasPanelSlot;
class UImage;

/**
 * Visual-only focus cursor controlled by the subsystem.
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUICursor : public UUserWidget
{
    GENERATED_BODY()

public:
    UNsUICursor(const FObjectInitializer& ObjectInitializer);

    /** Move the cursor so that it overlays the target widget. */
    UFUNCTION(BlueprintCallable, Category = "Cursor")
    void MoveToWidget(UWidget* const InTarget);

    /** Move the cursor to a viewport-relative screen-space position. */
    UFUNCTION(BlueprintCallable, Category = "Cursor")
    void MoveToScreenPosition(const FVector2D& InViewportPos);

    /** Whether the cursor should auto-hide when no target is available. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
    bool bHideWhenNoTarget = true;

    /** Target point on the focused widget (0-1 normalized). (0,0)=top-left. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
    FVector2D TargetAnchor;

    /** Local alignment of the cursor itself when positioned on screen. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
    FVector2D CursorAlignment;

    /** Cursor Horizontal Padding. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
    float HorizontalPadding;

    /** Default Slate brush used when the widget is constructed without designer content. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor", meta=(ShowOnlyInnerProperties))
    FSlateBrush CursorBrush;

protected:
    virtual void NativePreConstruct() override;

private:
    /** Ensures a minimal visual is available even without a BP-created widget tree. */
    void EnsureDefaultContent();

    /** Applies brush/padding settings to the generated widgets. */
    void RefreshDefaultStyle();

private:
    UPROPERTY(Transient)
    UCanvasPanel* RootCanvas;

    UPROPERTY(Transient)
    UImage* CursorImage;
};
