#pragma once

#include "CoreMinimal.h"
#include "NsUIFrameworkTypes.generated.h"

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
