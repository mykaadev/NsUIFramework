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

/**
 * Identifies what type of object is currently driving the retro cursor and focus state.
 */
UENUM(BlueprintType)
enum class ENsUIFocusKind : uint8
{
    /** Nothing is focused. */
    None,
    /** A UWidget is focused (traditional Slate/UMG path). */
    Widget,
    /** An in-world actor is focused (projected to screen space). */
    Actor,
    /** An arbitrary world-space point is focused (projected to screen space). */
    Location
};

/**
 * Unified focus target that can track widgets, actors, or raw world locations so the
 * retro cursor and selection behaviors can work across UI and 3D spaces.
 */
USTRUCT(BlueprintType)
struct FNsUIFocusTarget
{
    GENERATED_BODY()

    /** Type discriminator for the active focus target. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    ENsUIFocusKind Kind = ENsUIFocusKind::None;

    /** Widget reference when Kind == Widget. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    TWeakObjectPtr<UWidget> Widget;

    /** Actor reference when Kind == Actor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    TWeakObjectPtr<AActor> Actor;

    /** World-space focus position when Kind == Location (or actor override). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    FVector WorldLocation = FVector::ZeroVector;

    /** Cached screen-space position for the retro cursor to follow. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    FVector2D ScreenPosition = FVector2D::ZeroVector;

    /** Optional developer-facing label for debugging or logging. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    FName DebugName = NAME_None;

    /** Returns true when the target holds a usable reference for its kind. */
    bool IsValid() const
    {
        switch (Kind)
        {
        case ENsUIFocusKind::Widget:   return Widget.IsValid();
        case ENsUIFocusKind::Actor:    return Actor.IsValid();
        case ENsUIFocusKind::Location: return !WorldLocation.IsNearlyZero();
        default:                       return false;
        }
    }
};
