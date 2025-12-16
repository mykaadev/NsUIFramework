#pragma once

#include "CoreMinimal.h"
#include "NsUIFrameworkTypes.generated.h"

class AActor;
class UWidget;

/**
 * High-level screen strata used by the retro UI stack. Widgets are pushed to a layer to
 * control ordering and input routing (e.g., gameplay HUD vs. pause menus vs. modals).
 */
UENUM(BlueprintType)
enum class ENsUILayer : uint8
{
    Game,
    Menu,
    Modal
};

/**
 * Serialized snapshot of a layer's stack for save/restore flows. Exposed to Blueprints so
 * designer tooling can capture and reinstate UI state when swapping contexts.
 */
USTRUCT(BlueprintType)
struct FNsUILayerSnapshot
{
    GENERATED_BODY()

    FNsUILayerSnapshot()
        : Layer(ENsUILayer::Game)
        , FocusedIndex(INDEX_NONE)
    {
    }

    /** Layer the snapshot refers to. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    ENsUILayer Layer;

    /** Ordered list of widget class names currently on the stack. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    TArray<FString> WidgetNames;

    /** Index into WidgetNames that was focused when the snapshot was taken. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    int32 FocusedIndex;
};

/** Lightweight input source discriminator for cursor visibility heuristics. */
enum class ENsUIInputSource : uint8
{
    KeyboardGamepad,
    Mouse
};

