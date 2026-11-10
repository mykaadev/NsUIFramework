#pragma once

#include "CoreMinimal.h"
#include "NsUIFrameworkTypes.h"
#include "Types/SlateEnums.h"
#include "UObject/Object.h"
#include "NsUISelectionMode.generated.h"

/** Called when the selection of a world selection mode changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNsUISelectionChanged, class UNsUISelectionMode*, SelectionMode);

/**
 * Selects something other than the controls of a widget, such as world points or actors. One object for each open widget
 * While a selection mode is active, the widget root has the focus, and the mode gets the directions, the pointer, Confirm, and Back
 */
UCLASS(Abstract)
class NSUIFRAMEWORK_API UNsUISelectionMode : public UObject
{
    GENERATED_BODY()

// Functions
public:
    /** Sets the widget of the mode. The framework calls it when the widget opens */
    virtual void Initialize(class UUserWidget* InWidget);

    /** The widget got input. The default does nothing */
    virtual void Activate() {}

    /** Moves the selection in a direction. Returns true when the mode used the direction */
    virtual bool Navigate(EUINavigation Direction) { return false; }

    /** Selects the candidate at a viewport position. Returns true when a candidate is there */
    virtual bool PickAt(const FVector2D& ViewportPosition) { return false; }

    /** Uses Confirm. Returns true when the mode used it */
    virtual bool Confirm() { return false; }

    /** Uses Back. Returns true when the mode used it */
    virtual bool Cancel() { return false; }

    /** Gives the viewport position of the selection. Returns false when nothing is selected or the selection is not on screen */
    virtual bool GetSelectionPosition(FVector2D& OutViewportPosition) const { return false; }

    /** Returns the selection as text, for debug */
    virtual FString DescribeSelection() const { return FString(); }

    /** Returns the widget of the mode */
    UFUNCTION(BlueprintPure, Category = "NsUI")
    class UUserWidget* GetWidget() const { return Widget.Get(); }

protected:
    /** Returns the player controller of the widget, or null */
    class APlayerController* GetPlayerController() const;

// Variables
public:
    /** Called when the selection changes */
    UPROPERTY(BlueprintAssignable, Category = "NsUI")
    FNsUISelectionChanged OnSelectionChanged;

protected:
    /** The widget of the mode */
    UPROPERTY(Transient)
    TWeakObjectPtr<class UUserWidget> Widget;
};

/** The base of the modes that select world positions. A direction moves to the nearest candidate in that direction on screen */
UCLASS(Abstract)
class NSUIFRAMEWORK_API UNsUIWorldSelectionMode : public UNsUISelectionMode
{
    GENERATED_BODY()

// Functions
public:
    /** Constructor */
    UNsUIWorldSelectionMode();

    //~ Begin UNsUISelectionMode Interface
    virtual void Activate() override;
    virtual bool Navigate(EUINavigation Direction) override;
    virtual bool PickAt(const FVector2D& ViewportPosition) override;
    virtual bool GetSelectionPosition(FVector2D& OutViewportPosition) const override;
    //~ End UNsUISelectionMode Interface

    /** Selects a candidate by its index. INDEX_NONE clears the selection */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    void SelectIndex(int32 Index);

    /** Returns the index of the selected candidate, or INDEX_NONE */
    UFUNCTION(BlueprintPure, Category = "NsUI")
    int32 GetSelectedIndex() const { return SelectedIndex; }

protected:
    /** Returns the number of candidates */
    virtual int32 GetCandidateCount() const PURE_VIRTUAL(UNsUIWorldSelectionMode::GetCandidateCount, return 0;);

    /** Gives the world position of a candidate. Returns false when the candidate has no position now */
    virtual bool GetCandidatePosition(int32 Index, FVector& OutPosition) const PURE_VIRTUAL(UNsUIWorldSelectionMode::GetCandidatePosition, return false;);

    /** Gives the viewport position of a candidate. Returns false when it is not on screen */
    bool ProjectCandidate(int32 Index, FVector2D& OutViewportPosition) const;

    /** Clears a selection that is past the end of the candidates */
    void ClampSelection();

// Variables
public:
    /** True selects the candidate nearest to the screen center when the widget gets input with no selection. False keeps no selection until the player moves */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NsUI")
    bool bSelectOnActivate;

protected:
    /** The index of the selected candidate, or INDEX_NONE */
    int32 SelectedIndex;
};

/** Selects one actor of a list */
UCLASS()
class NSUIFRAMEWORK_API UNsUIActorSelectionMode : public UNsUIWorldSelectionMode
{
    GENERATED_BODY()

// Functions
public:
    //~ Begin UNsUISelectionMode Interface
    virtual FString DescribeSelection() const override;
    //~ End UNsUISelectionMode Interface

    /** Sets the actors to select from. The selection resets */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    void SetCandidates(const TArray<class AActor*>& InCandidates);

    /** Returns the selected actor, or null */
    UFUNCTION(BlueprintPure, Category = "NsUI")
    class AActor* GetSelectedActor() const;

protected:
    //~ Begin UNsUIWorldSelectionMode Interface
    virtual int32 GetCandidateCount() const override { return Candidates.Num(); }
    virtual bool GetCandidatePosition(int32 Index, FVector& OutPosition) const override;
    //~ End UNsUIWorldSelectionMode Interface

// Variables
private:
    /** The actors to select from. The mode does not keep them alive */
    TArray<TWeakObjectPtr<class AActor>> Candidates;
};

/** Selects one world point of a list */
UCLASS()
class NSUIFRAMEWORK_API UNsUILocationSelectionMode : public UNsUIWorldSelectionMode
{
    GENERATED_BODY()

// Functions
public:
    //~ Begin UNsUISelectionMode Interface
    virtual FString DescribeSelection() const override;
    //~ End UNsUISelectionMode Interface

    /** Sets the points to select from. A point with no id or a repeated id is left out. The selection resets */
    UFUNCTION(BlueprintCallable, Category = "NsUI")
    void SetCandidates(const TArray<FNsUILocation>& InCandidates);

    /** Returns the id of the selected point, or None */
    UFUNCTION(BlueprintPure, Category = "NsUI")
    FName GetSelectedId() const;

protected:
    //~ Begin UNsUIWorldSelectionMode Interface
    virtual int32 GetCandidateCount() const override { return Candidates.Num(); }
    virtual bool GetCandidatePosition(int32 Index, FVector& OutPosition) const override;
    //~ End UNsUIWorldSelectionMode Interface

// Variables
private:
    /** The points to select from */
    UPROPERTY(Transient)
    TArray<FNsUILocation> Candidates;
};
