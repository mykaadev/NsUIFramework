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

UENUM(BlueprintType)
enum class ENsUIFocusKind : uint8
{
    None,
    Widget,
    Actor,
    Location
};

USTRUCT(BlueprintType)
struct FNsUIFocusTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    ENsUIFocusKind Kind = ENsUIFocusKind::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    TWeakObjectPtr<UWidget> Widget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    TWeakObjectPtr<AActor> Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    FVector2D ScreenPosition = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
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
