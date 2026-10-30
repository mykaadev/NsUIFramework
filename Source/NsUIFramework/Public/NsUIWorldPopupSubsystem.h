#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"
#include "Tickable.h"
#include "Templates/SubclassOf.h"
#include "NsUIWorldPopupSubsystem.generated.h"

class AActor;
class UNsUIWorldPopupWidget;

/** One popup that shows now */
USTRUCT()
struct FNsUIWorldPopupEntry
{
    GENERATED_BODY()

    /** The widget of the popup */
    UPROPERTY(Transient)
    TObjectPtr<UNsUIWorldPopupWidget> Widget;

    /** The actor that the popup follows, or null for a fixed point */
    TWeakObjectPtr<AActor> FollowActor;

    /** The world point of the popup, or the offset from the follow actor */
    FVector Location = FVector::ZeroVector;

    /** True when the popup follows an actor */
    bool bFollows = false;

    /** The time in seconds since the popup started */
    float Age = 0.f;
};

/**
 * Floating texts over world points for one local player, for example "+3 coins" or "GUILTY". Each popup is a pooled
 * widget on the player screen. It rises, fades, and hides when it is far or behind a wall
 */
UCLASS()
class NSUIFRAMEWORK_API UNsUIWorldPopupSubsystem : public ULocalPlayerSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:
    //~ Begin ULocalPlayerSubsystem
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //~ End ULocalPlayerSubsystem

    //~ Begin FTickableGameObject
    virtual void Tick(float DeltaTime) override;
    virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }
    virtual bool IsTickable() const override { return !Active.IsEmpty(); }
    virtual TStatId GetStatId() const override;
    virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }
    //~ End FTickableGameObject

    /** Shows a popup over a world point. With a follow actor, the location is an offset from the actor. Returns the popup, or null */
    UFUNCTION(BlueprintCallable, Category="NsUI|World Popup", meta=(AdvancedDisplay="FollowActor"))
    UNsUIWorldPopupWidget* ShowPopup(TSubclassOf<UNsUIWorldPopupWidget> PopupClass, const FText& Text, FVector Location, FLinearColor Color, AActor* FollowActor = nullptr);

    /** Ends every popup at once */
    UFUNCTION(BlueprintCallable, Category="NsUI|World Popup")
    void ClearPopups();

    /** The popups that show now */
    UFUNCTION(BlueprintPure, Category="NsUI|World Popup")
    int32 GetActivePopupCount() const { return Active.Num(); }

private:
    /** Moves one popup to its place on the screen. Returns false when the popup is over */
    bool UpdatePopup(FNsUIWorldPopupEntry& Entry, const class APlayerController* Player, float DeltaTime) const;

    /** Ends one popup and keeps its widget for the next popup of the same class */
    void Release(int32 Index);

    /** Drops the popups of a world that ends */
    void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

    /** The popups that show now, oldest first */
    UPROPERTY(Transient)
    TArray<FNsUIWorldPopupEntry> Active;

    /** Widgets that wait for the next popup */
    UPROPERTY(Transient)
    TArray<TObjectPtr<UNsUIWorldPopupWidget>> Pool;

    /** The handle of the world cleanup listener */
    FDelegateHandle CleanupHandle;
};
