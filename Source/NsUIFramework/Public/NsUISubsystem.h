#pragma once

#include "NsUIFrameworkTypes.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Tickable.h"
#include "NsUISubsystem.generated.h"

class UUserWidget;
class UWidget;
class UNsUICursor;
class UNsUILayout;
class SWidget;
class FNavigationConfig;
class UGameViewportClient;

/** Owns screen interaction for one local player. */
UCLASS()
class NSUIFRAMEWORK_API UNsUISubsystem : public ULocalPlayerSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:
    //~ Begin ULocalPlayerSubsystem
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //~ End ULocalPlayerSubsystem

    //~ Begin FTickableGameObject
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickableWhenPaused() const override { return true; }
    virtual TStatId GetStatId() const override;
    virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }
    //~ End FTickableGameObject

    /** Create and own a screen on the requested layer */
    UFUNCTION(BlueprintCallable, Category="NsUI")
    UUserWidget* OpenScreen(TSubclassOf<UUserWidget> ScreenClass, UObject* Context, ENsUILayer Layer = ENsUILayer::Menu);

    /** Close an owned screen; external widgets remain with their creator */
    UFUNCTION(BlueprintCallable, Category="NsUI")
    void CloseScreen(UUserWidget* Widget);

    /** Close the owned screens on one layer */
    UFUNCTION(BlueprintCallable, Category="NsUI")
    void CloseAll(ENsUILayer Layer);

private:
    friend class FNsUISlateInputRouter;
    friend class FNsUIInspector;
    struct FScreenEntry;

    void AddEntry(UUserWidget* Widget, UObject* Context, ENsUILayer Layer, bool bOwned);
    void RemoveEntry(const TSharedPtr<FScreenEntry>& Entry, bool bRemoveWidget);
    void OnWidgetAdded(UWidget* Widget, ULocalPlayer* Player);
    void OnWidgetRemoved(UWidget* Widget);
    void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);
    void Reconcile();
    void RecoverSelection();
    void EnsurePresentation();
    void RemovePresentation();
    void ApplyInputPolicy();
    void UpdateNavigationConfig();
    void SetSuspended(bool bSuspend);
    void ObserveFocus(const TSharedPtr<SWidget>& Widget);
    bool Contains(const TSharedPtr<SWidget>& Widget) const;
    bool IsScreenPopup(const TSharedPtr<SWidget>& Widget) const;
    bool IsGameFocus(const TSharedPtr<SWidget>& Widget) const;
    bool RouteAction(bool bConfirm);
    bool UsesNativeNavigation() const;
    bool ObservePointer(const FVector2D& AbsolutePosition);
    bool RouteNavigate(EUINavigation Direction, bool bGamepad);
    int32 GetUserIndex() const;
    FString DescribeState() const;

    UPROPERTY(Transient)
    TObjectPtr<UNsUILayout> Layout;

    UPROPERTY(Transient)
    TObjectPtr<UNsUICursor> Cursor;

    TArray<TSharedPtr<FScreenEntry>> Screens;
    TSharedPtr<FScreenEntry> Active;
    TWeakObjectPtr<UUserWidget> PolicyOwner;
    FDelegateHandle AddedHandle;
    FDelegateHandle RemovedHandle;
    FDelegateHandle CleanupHandle;
    bool bSuspended = false;
    bool bRecover = false;
    bool bShuttingDown = false;
    bool bInitialDiscovery = true;
    bool bLogical = false;
    bool bReconciling = false;
    bool bReconcileAgain = false;
    float RefreshElapsed = 0.f;
    FString RecoveryReason;
    TSharedPtr<FNavigationConfig> PreviousNavigation;
    TSharedPtr<FNavigationConfig> Navigation;
    TSharedPtr<SWidget> Presentation;
    TWeakObjectPtr<UGameViewportClient> PresentationViewport;
    int32 PresentationZOrder = 0;
};
