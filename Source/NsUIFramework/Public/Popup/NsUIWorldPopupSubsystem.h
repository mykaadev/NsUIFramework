#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Templates/SubclassOf.h"
#include "Tickable.h"
#include "NsUIWorldPopupSubsystem.generated.h"

/** One popup that shows. Only the world popup subsystem makes and changes entries */
USTRUCT()
struct FNsUIWorldPopupEntry
{
    GENERATED_BODY()

    /** The popup widget */
    UPROPERTY()
    TObjectPtr<class UNsUIWorldPopupWidget> Widget;

    /** The actor that the popup follows, or none */
    TWeakObjectPtr<class AActor> FollowActor;

    /** The world point, or the offset from the follow actor */
    FVector Point = FVector::ZeroVector;

    /** The time since the popup started */
    float Age = 0.f;

    /** True after the Disappear animation started */
    bool bDisappearing = false;
};

/** Shows floating texts over world points for one local player, for example "+3 coins". Only this player sees them */
UCLASS()
class NSUIFRAMEWORK_API UNsUIWorldPopupSubsystem : public ULocalPlayerSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

// Functions
public:
    //~ Begin USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //~ End USubsystem Interface

    //~ Begin FTickableGameObject Interface
    virtual void Tick(float DeltaTime) override;
    virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }
    virtual bool IsTickable() const override { return Active.Num() > 0; }
    virtual TStatId GetStatId() const override;
    virtual UWorld* GetTickableGameObjectWorld() const override;
    //~ End FTickableGameObject Interface

    /** Shows a popup. With a follow actor, the location is an offset from the actor. Without one, it is a world point */
    UFUNCTION(BlueprintCallable, Category = "NsUI|Popup")
    void ShowPopup(TSubclassOf<class UNsUIWorldPopupWidget> PopupClass, const FText& Text, FVector Location, FLinearColor Color, class AActor* FollowActor = nullptr);

    /** Ends every popup at once */
    UFUNCTION(BlueprintCallable, Category = "NsUI|Popup")
    void ClearPopups();

    /** Returns the number of popups that show */
    UFUNCTION(BlueprintPure, Category = "NsUI|Popup")
    int32 GetActivePopupCount() const { return Active.Num(); }

private:
    /** Moves a popup and gives its screen position. Returns false when the popup must hide this frame */
    bool UpdatePopup(const FNsUIWorldPopupEntry& Entry, const class APlayerController* Player, FVector2D& OutScreenPosition) const;

    /** Takes a popup off the screen and keeps it for reuse */
    void Release(int32 Index);

    /** Handles the cleanup of a world */
    void HandleWorldCleanup(class UWorld* World, bool bSessionEnded, bool bCleanupResources);

// Variables
private:
    /** The popups that show */
    UPROPERTY(Transient)
    TArray<FNsUIWorldPopupEntry> Active;

    /** Popup widgets that wait for reuse */
    UPROPERTY(Transient)
    TArray<TObjectPtr<class UNsUIWorldPopupWidget>> Pool;

    /** The binding of the world cleanup event */
    FDelegateHandle WorldCleanupHandle;
};
