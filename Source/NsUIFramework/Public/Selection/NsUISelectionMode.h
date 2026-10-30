#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Types/SlateEnums.h"
#include "UObject/Object.h"
#include "NsUISelectionMode.generated.h"

class UUserWidget;
class UWidget;
class APlayerController;
class AActor;
class SWidget;

/** Selection state owned by one screen, including while suspended. */
UCLASS(Abstract, NotBlueprintable)
class NSUIFRAMEWORK_API UNsUISelectionMode : public UObject
{
    GENERATED_BODY()

public:
    virtual void Initialize(UUserWidget* InScreen);
    virtual void Refresh() {}
    virtual bool Restore(UWidget* Preferred) PURE_VIRTUAL(UNsUISelectionMode::Restore, return false;);
    virtual bool HasSelection() const PURE_VIRTUAL(UNsUISelectionMode::HasSelection, return false;);
    virtual bool Navigate(EUINavigation Direction) { return false; }
    virtual bool UsesNativeNavigation() const { return false; }
    virtual bool ObservePointer(const FVector2D& Position) { return false; }
    virtual bool Confirm() { return false; }
    virtual bool Cancel() { return false; }
    virtual UWidget* GetFocusTarget() const { return nullptr; }
    virtual bool ObserveFocus(const TSharedPtr<SWidget>& Widget) { return false; }
    virtual bool GetCursorPosition(FVector2D& Position) const PURE_VIRTUAL(UNsUISelectionMode::GetCursorPosition, return false;);
    virtual FString DescribeSelection() const { return FString(); }

protected:
    APlayerController* GetPlayerController() const;

    UPROPERTY(Transient)
    TWeakObjectPtr<UUserWidget> Screen;
};

/** Discovers controls inside the screen and nested widget trees. */
UCLASS()
class NSUIFRAMEWORK_API UNsUIUMGSelectionMode : public UNsUISelectionMode
{
    GENERATED_BODY()

public:
    virtual bool UsesNativeNavigation() const override { return true; }
    virtual void Refresh() override;
    virtual bool Restore(UWidget* Preferred) override;
    virtual bool HasSelection() const override;
    virtual UWidget* GetFocusTarget() const override;
    virtual bool ObserveFocus(const TSharedPtr<SWidget>& Widget) override;
    virtual bool GetCursorPosition(FVector2D& Position) const override;
    virtual FString DescribeSelection() const override;

private:
    TArray<TWeakObjectPtr<UWidget>> Candidates;
    TWeakObjectPtr<UWidget> Selected;
    TWeakPtr<SWidget> NativePopupFocus;
};

/** Shared directional selection for explicit world candidates. */
UCLASS(Abstract)
class NSUIFRAMEWORK_API UNsUIWorldSelectionMode : public UNsUISelectionMode
{
    GENERATED_BODY()

public:
    virtual bool Restore(UWidget* Preferred) override;
    virtual bool HasSelection() const override;
    virtual bool Navigate(EUINavigation Direction) override;
    virtual bool ObservePointer(const FVector2D& Position) override;
    virtual bool GetCursorPosition(FVector2D& Position) const override;

protected:
    virtual int32 GetCandidateCount() const PURE_VIRTUAL(UNsUIWorldSelectionMode::GetCandidateCount, return 0;);
    virtual bool GetCandidatePosition(int32 Index, FVector& Position) const PURE_VIRTUAL(UNsUIWorldSelectionMode::GetCandidatePosition, return false;);
    bool Project(int32 Index, FVector2D& Position) const;
    int32 SelectedIndex = INDEX_NONE;
};

UCLASS()
class NSUIFRAMEWORK_API UNsUIActorSelectionMode : public UNsUIWorldSelectionMode
{
    GENERATED_BODY()

public:
    void SetCandidates(const TArray<TWeakObjectPtr<AActor>>& InCandidates);
    AActor* GetSelectedActor() const;
    virtual FString DescribeSelection() const override;

protected:
    virtual int32 GetCandidateCount() const override;
    virtual bool GetCandidatePosition(int32 Index, FVector& Position) const override;

private:
    TArray<TWeakObjectPtr<AActor>> Candidates;
};

struct NSUIFRAMEWORK_API FNsUILocation
{
    FName Id;
    FVector Position = FVector::ZeroVector;
};

UCLASS()
class NSUIFRAMEWORK_API UNsUILocationSelectionMode : public UNsUIWorldSelectionMode
{
    GENERATED_BODY()

public:
    void SetCandidates(const TArray<FNsUILocation>& InCandidates);
    FName GetSelectedId() const;
    virtual FString DescribeSelection() const override;

protected:
    virtual int32 GetCandidateCount() const override;
    virtual bool GetCandidatePosition(int32 Index, FVector& Position) const override;

private:
    TArray<FNsUILocation> Candidates;
};
