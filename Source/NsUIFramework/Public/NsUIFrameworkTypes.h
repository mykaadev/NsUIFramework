#pragma once

#include "CoreMinimal.h"
#include "NsUIFrameworkTypes.generated.h"

class UWidget;
class UNsUISelectionMode;

UENUM(BlueprintType)
enum class ENsUILayer : uint8 { Game, Menu, Modal };

UENUM(BlueprintType)
enum class ENsUIGameplayPolicy : uint8 { Automatic, Allow, Block };

UENUM(BlueprintType)
enum class ENsUISelectionPolicy : uint8 { Automatic, Logical };

USTRUCT(BlueprintType)
struct NSUIFRAMEWORK_API FNsUIScreenConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NsUI")
    ENsUILayer Layer = ENsUILayer::Menu;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NsUI")
    bool bInteractive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NsUI")
    ENsUIGameplayPolicy Gameplay = ENsUIGameplayPolicy::Automatic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NsUI")
    TSubclassOf<UNsUISelectionMode> SelectionMode;
};

USTRUCT(BlueprintType)
struct NSUIFRAMEWORK_API FNsUISelectionRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NsUI")
    ENsUISelectionPolicy Policy = ENsUISelectionPolicy::Automatic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NsUI")
    TObjectPtr<UWidget> PreferredTarget = nullptr;
};
