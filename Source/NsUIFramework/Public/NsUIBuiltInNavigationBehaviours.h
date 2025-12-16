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
    virtual void OnConfirm_Implementation() override;
    virtual void OnCancel_Implementation() override;
};

/** Navigation strategy that cycles through a supplied actor list. */
UCLASS(Blueprintable)
class NSUIFRAMEWORK_API UNsUIActorNavigationBehaviour : public UNsUINavigationBehaviour
{
    GENERATED_BODY()

public:
    /** Replace the actor list used for navigation. */
    UFUNCTION(BlueprintCallable, Category = "NsUI|Navigation|Actors")
    void SetActors(const TArray<AActor*>& InActors);

    /** Clear any stored actor list. */
    UFUNCTION(BlueprintCallable, Category = "NsUI|Navigation|Actors")
    void ClearActors();

    /** Current actor under navigation control, if any. */
    UFUNCTION(BlueprintPure, Category = "NsUI|Navigation|Actors")
    AActor* GetCurrentActor() const;

    virtual void OnNavigate_Implementation(EUINavigation Direction) override;
    virtual void OnEnter_Implementation() override;
    virtual void OnTick_Implementation(float DeltaTime) override;
    virtual void OnConfirm_Implementation() override;
    virtual void OnCancel_Implementation() override;

private:
    void FocusActorAtIndex(int32 Index);

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> Actors;

    int32 CurrentIndex = INDEX_NONE;
};

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

