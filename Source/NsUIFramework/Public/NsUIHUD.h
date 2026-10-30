#pragma once

#include "GameFramework/HUD.h"
#include "NsUIFrameworkTypes.h"
#include "Templates/SubclassOf.h"
#include "NsUIHUD.generated.h"

class UUserWidget;

/** One screen that a HUD opens when it starts */
USTRUCT(BlueprintType)
struct NSUIFRAMEWORK_API FNsUIStartScreen
{
    GENERATED_BODY()

    /** Widget class of the screen */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NsUI")
    TSubclassOf<UUserWidget> Screen;

    /** Layer of the screen. Game keeps gameplay input, Menu shows the cursor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NsUI")
    ENsUILayer Layer = ENsUILayer::Game;
};

/**
 * HUD that opens its start screens for the local player. The game mode Blueprint picks the HUD class,
 * and the HUD Blueprint picks the screens, for example a title menu or the gameplay HUD
 */
UCLASS(Blueprintable)
class NSUIFRAMEWORK_API ANsUIHUD : public AHUD
{
    GENERATED_BODY()

public:
    //~ Begin AActor Interface
    virtual void BeginPlay() override;
    //~ End AActor Interface

    /** Return the open start screen of a class, or null */
    UFUNCTION(BlueprintPure, Category="NsUI", meta=(DeterminesOutputType="ScreenClass"))
    UUserWidget* GetStartScreen(TSubclassOf<UUserWidget> ScreenClass) const;

    /** Screens that open when the HUD starts, in this order */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NsUI")
    TArray<FNsUIStartScreen> StartScreens;

private:
    /** Start screens that are open */
    UPROPERTY(Transient)
    TArray<TObjectPtr<UUserWidget>> OpenScreens;
};
