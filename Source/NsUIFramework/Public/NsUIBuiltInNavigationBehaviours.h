#pragma once

#include "NsUINavigationBehaviour.h"
#include "NsUIBuiltInNavigationBehaviours.generated.h"

/** Default UI navigation that defers to NsUI's widget spatial search and Slate confirm/cancel. */
UCLASS(Blueprintable)
class NSUIFRAMEWORK_API UNsUIWidgetNavigationBehaviour : public UNsUINavigationBehaviour
{
    GENERATED_BODY()

public:
    virtual void OnNavigate_Implementation(EUINavigation Direction) override;
    virtual void OnConfirm_Implementation(const FNsUIFocusTarget& Target) override;
    virtual void OnCancel_Implementation() override;
};

/** Navigation strategy that cycles through actor focus targets only. */
UCLASS(Blueprintable)
class NSUIFRAMEWORK_API UNsUIActorNavigationBehaviour : public UNsUINavigationBehaviour
{
    GENERATED_BODY()

public:
    virtual void OnNavigate_Implementation(EUINavigation Direction) override;
};

/** Navigation strategy that cycles through world-location focus targets only. */
UCLASS(Blueprintable)
class NSUIFRAMEWORK_API UNsUILocationNavigationBehaviour : public UNsUINavigationBehaviour
{
    GENERATED_BODY()

public:
    virtual void OnNavigate_Implementation(EUINavigation Direction) override;
};

