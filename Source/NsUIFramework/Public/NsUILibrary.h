// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NsUILibrary.generated.h"

class UUserWidget;
class UWidget;

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

    /** Convenience helper to register actors for selection and bind callbacks. */
    static void RegisterActorsForSelection(const TArray<AActor*>& Actors,
                                           const FNsUIActorDelegate::FDelegate& OnConfirm,
                                           const FNsUIActorDelegate::FDelegate& OnCancel = FNsUIActorDelegate::FDelegate(),
                                           const FNsUIActorDelegate::FDelegate& OnToggle = FNsUIActorDelegate::FDelegate());
};
