#pragma once

#include "CoreMinimal.h"
#include "NsUIFrameworkTypes.generated.h"

class AActor;
class UWidget;

UENUM(BlueprintType)
enum class ENsUILayer : uint8
{
    Game,
    Menu,
    Modal
};

USTRUCT(BlueprintType)
struct FNsUILayerSnapshot
{
    GENERATED_BODY()

    FNsUILayerSnapshot()
        : Layer(ENsUILayer::Game)
        , FocusedIndex(INDEX_NONE)
    {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    ENsUILayer Layer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    TArray<FString> WidgetNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    int32 FocusedIndex;
};

enum class ENsUIInputSource : uint8
{
    KeyboardGamepad,
    Mouse
};

UENUM()
enum class ENsUIFocusKind : uint8
{
    None,
    Widget,
    Actor,
    Location
};

USTRUCT()
struct FNsUIFocusTarget
{
    GENERATED_BODY()

    UPROPERTY()
    ENsUIFocusKind Kind = ENsUIFocusKind::None;

    UPROPERTY()
    TWeakObjectPtr<UWidget> Widget;

    UPROPERTY()
    TWeakObjectPtr<AActor> Actor;

    UPROPERTY()
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY()
    FVector2D ScreenPosition = FVector2D::ZeroVector;

    UPROPERTY()
    FName DebugName = NAME_None;

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
