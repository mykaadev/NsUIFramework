// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "NsUILibrary.generated.h"

class UUserWidget;
class UWidget;
class UNsUIWidget;

/** Built-in layer slots that the default layout creates. */
UENUM(BlueprintType)
enum class ENsUILayer : uint8
{
    Menu,
    Modal
};

/** Delegate used to notify about actor interactions */
DECLARE_MULTICAST_DELEGATE_OneParam(FNsUIActorDelegate, AActor*);

/** Holds info for a world actor registered as a focus target. */
struct FNsUIActorFocusInfo
{
    TWeakObjectPtr<AActor> Actor;
    /** Actor's projected bounds in viewport pixel coordinates */
    FBox2D ScreenBounds;
};

/**
 * Helper library for Ns UI framework.
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUILibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Gather all focusable descendants of a widget. */
    static void GatherAllNavigableDescendants(const UUserWidget* InRootWidget, TArray<UWidget*>& InOutNavigables);

    /** Find the first focusable descendant of a widget. */
    static UWidget* FindFirstFocusableDescendant(const UUserWidget* InRootWidget);

    /** Convenience wrapper to push a widget class onto a layer slot. */
    UFUNCTION(BlueprintCallable, Category = "NsUI|Layers")
    static UUserWidget* PushWidgetToLayer(TSubclassOf<UUserWidget> WidgetClass, ENsUILayer Layer);

    /** Convenience wrapper to push an existing widget instance onto a layer slot. */
    UFUNCTION(BlueprintCallable, Category = "NsUI|Layers")
    static bool PushWidgetInstanceToLayer(UUserWidget* Widget, ENsUILayer Layer);

    /** Pop the top widget off the given layer. */
    UFUNCTION(BlueprintCallable, Category = "NsUI|Layers")
    static void PopLayer(ENsUILayer Layer);

    /** Pop all widgets back to the root entry on the given layer. */
    UFUNCTION(BlueprintCallable, Category = "NsUI|Layers")
    static void PopLayerToRoot(ENsUILayer Layer);

    /** Convenience helper to register actors for selection and bind callbacks. */
    static void RegisterActorsForSelection(const TArray<AActor*>& Actors,
                                           const FNsUIActorDelegate::FDelegate& OnConfirm,
                                           const FNsUIActorDelegate::FDelegate& OnCancel = FNsUIActorDelegate::FDelegate(),
                                           const FNsUIActorDelegate::FDelegate& OnToggle = FNsUIActorDelegate::FDelegate());
};
