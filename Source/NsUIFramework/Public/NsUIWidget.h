// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "INsUINavigation.h"
#include "NsUIWidget.generated.h"

/**
 * Base widget for the Ns UI framework. Focusable by default and capable of providing
 * neighbor metadata for navigation fallbacks.
 */
UCLASS(Abstract, Blueprintable)
class NSUIFRAMEWORK_API UNsUIWidget : public UUserWidget, public INsUINavigation
{
    GENERATED_BODY()

public:
    UNsUIWidget(const FObjectInitializer& ObjectInitializer);

    //~ Begin UUserWidget Interface
    virtual void NativeOnInitialized() override;
    virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;
    virtual void NativeOnFocusLost(const FFocusEvent& InFocusEvent) override;
    virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
    virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
    //~ End UUserWidget Interface

    /** Neighbor lookup used by subsystem fallbacks. */
    UFUNCTION(BlueprintCallable, Category = "Navigation")
    UWidget* GetNeighbor(const EUINavigation Direction) const;

    /** Expose soft references so designers can author explicit neighbors. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    TSoftObjectPtr<UWidget> UpNeighbor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    TSoftObjectPtr<UWidget> DownNeighbor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    TSoftObjectPtr<UWidget> LeftNeighbor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    TSoftObjectPtr<UWidget> RightNeighbor;

protected:
    /** Sync explicit navigation data with the engine based on neighbor pointers. */
    void ApplyNeighborNavigation();
};
