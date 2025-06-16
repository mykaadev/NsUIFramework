#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "INsUINavigation.generated.h"

UINTERFACE(BlueprintType)
class NSUIFRAMEWORK_API UNsUINavigation : public UInterface
{
    GENERATED_BODY()
};

/** UI Navigation Interface */
class NSUIFRAMEWORK_API INsUINavigation
{
    GENERATED_BODY()

// Functions
public:

    /** Get up Neighbor */
    /** On enter the interaction radius */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    class UWidget* GetUpNeighbor();
    virtual class UWidget* GetUpNeighbor_Implementation() = 0;

    /** Get down Neighbor */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    class UWidget* GetDownNeighbor();
    virtual class UWidget* GetDownNeighbor_Implementation() = 0;

    /** Get left neighbor */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    class UWidget* GetLeftNeighbor();
    virtual class UWidget* GetLeftNeighbor_Implementation() = 0;

    /** Get right neighbor */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    class UWidget* GetRightNeighbor();
    virtual class UWidget* GetRightNeighbor_Implementation() = 0;

    /** Handle UI Confirm */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void HandleUIConfirm();
    virtual void HandleUIConfirm_Implementation() = 0;

    /** Handle UI Cancel */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void HandleUICancel();
    virtual void HandleUICancel_Implementation() = 0;

    /** Handle UI Cancel */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void OnWidgetPushed();
    virtual void OnWidgetPushed_Implementation() = 0;
};
