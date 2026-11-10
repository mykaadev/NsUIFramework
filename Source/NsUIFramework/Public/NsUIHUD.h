#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "NsUIFrameworkTypes.h"
#include "Templates/SubclassOf.h"
#include "NsUIHUD.generated.h"

/** One widget that the HUD opens when play starts */
USTRUCT(BlueprintType)
struct NSUIFRAMEWORK_API FNsUIStartWidget
{
    GENERATED_BODY()

    /** The widget class to open */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NsUI")
    TSubclassOf<class UUserWidget> Widget;

    /** The layer of the widget */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NsUI")
    ENsUILayer Layer = ENsUILayer::Game;
};

/** A HUD that opens a list of widgets when play starts, and closes them when play ends */
UCLASS(Blueprintable)
class NSUIFRAMEWORK_API ANsUIHUD : public AHUD
{
    GENERATED_BODY()

// Functions
public:
    //~ Begin AActor Interface
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //~ End AActor Interface

    /** Returns the open start widget of a class, or null */
    UFUNCTION(BlueprintPure, Category = "NsUI", meta = (DeterminesOutputType = "WidgetClass"))
    class UUserWidget* GetStartWidget(TSubclassOf<class UUserWidget> WidgetClass) const;

// Variables
public:
    /** The widgets to open on BeginPlay, in order */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NsUI")
    TArray<FNsUIStartWidget> StartWidgets;

private:
    /** The widgets that this HUD opened. The HUD closes the ones that are still open in EndPlay */
    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<class UUserWidget>> OpenedWidgets;
};
