// Copyright (C) 2025 nulled.softworks. All rights reserved.

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

// Functions
public:

    /** Constructor */
    UNsUICursor(const FObjectInitializer& ObjectInitializer);

    /** Move the cursor so that it overlays the target widget. */
    UFUNCTION(BlueprintCallable, Category="Cursor")
    void MoveToWidget(class UWidget* const InTarget);

    /** Move the cursor to a viewport-relative screen-space position. */
    UFUNCTION(BlueprintCallable, Category="Cursor")
    void MoveToScreenPosition(const FVector2D& InViewportPos);

// Variables
public:

    /** Target point on the focused widget (0-1 normalized). (0,0)=top-left. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cursor")
    FVector2D TargetAnchor;

    /** Local alignment of the cursor itself when positioned on screen. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cursor")
    FVector2D CursorAlignment;

    /** Cursor Horizontal Padding. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cursor")
    float HorizontalPadding;

};

