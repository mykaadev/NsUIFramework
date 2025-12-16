#pragma once

#include "NsUINavigationBehaviour.h"
#include "Navigation/NsUILocationNavigationBehaviour.generated.h"

/** Navigation strategy stub for world-location driven navigation. */
UCLASS(Blueprintable)
class NSUIFRAMEWORK_API UNsUILocationNavigationBehaviour : public UNsUINavigationBehaviour
{
    GENERATED_BODY()

public:
    virtual void OnNavigate_Implementation(EUINavigation Direction) override;
    virtual void OnConfirm_Implementation() override;
    virtual void OnCancel_Implementation() override;
};
