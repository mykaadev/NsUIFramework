#include "Selection/NsUISelectionMode.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/PanelWidget.h"
#include "Components/ListViewBase.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Actor.h"
#include "Widgets/SWidget.h"

namespace
{
void Collect(UWidget* Widget, TArray<TWeakObjectPtr<UWidget>>& Out)
{
    if (!IsValid(Widget) || !Widget->IsVisible() || !Widget->GetIsEnabled())
    {
        return;
    }
    const int32 PreviousCount = Out.Num();
    // Descendants receive native input before a user widget can act as a control.
    if (UUserWidget* User = Cast<UUserWidget>(Widget))
    {
        if (User->WidgetTree)
        {
            Collect(User->WidgetTree->RootWidget, Out);
        }
    }
    else if (UListViewBase* List = Cast<UListViewBase>(Widget))
    {
        for (UUserWidget* Entry : List->GetDisplayedEntryWidgets())
        {
            Collect(Entry, Out);
        }
    }
    else if (UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
    {
        for (UWidget* Child : Panel->GetAllChildren())
        {
            Collect(Child, Out);
        }
    }
    const TSharedPtr<SWidget> Slate = Widget->GetCachedWidget();
    const bool bContainerWithControls = Widget->IsA<UUserWidget>() && Out.Num() > PreviousCount;
    if (!bContainerWithControls && Slate && Slate->SupportsKeyboardFocus() && Slate->IsEnabled())
    {
        Out.AddUnique(Widget);
    }
}
}

void UNsUISelectionMode::Initialize(UUserWidget* InScreen)
{
    Screen = InScreen;
}

APlayerController* UNsUISelectionMode::GetPlayerController() const
{
    return Screen.IsValid() ? Screen->GetOwningPlayer() : nullptr;
}

void UNsUIUMGSelectionMode::Refresh()
{
    Candidates.Reset();
    Collect(Screen.Get(), Candidates);
}

bool UNsUIUMGSelectionMode::Restore(UWidget* Preferred)
{
    Refresh();
    if (HasSelection())
    {
        return true;
    }
    Selected = Candidates.Contains(Preferred) ? Preferred : Candidates.IsEmpty() ? nullptr : Candidates[0].Get();
    return HasSelection();
}

bool UNsUIUMGSelectionMode::HasSelection() const
{
    return Selected.IsValid() && Candidates.Contains(Selected) && Selected->IsVisible() && Selected->GetIsEnabled();
}

UWidget* UNsUIUMGSelectionMode::GetFocusTarget() const
{
    return HasSelection() ? Selected.Get() : nullptr;
}

bool UNsUIUMGSelectionMode::ObserveFocus(const TSharedPtr<SWidget>& Widget)
{
    Refresh();
    for (const TWeakObjectPtr<UWidget>& Candidate : Candidates)
    {
        TSharedPtr<SWidget> Current = Widget;
        while (Current)
        {
            if (Candidate.IsValid() && Candidate->GetCachedWidget() == Current)
            {
                Selected = Candidate;
                NativePopupFocus.Reset();
                return true;
            }
            Current = Current->GetParentWidget();
        }
    }
    if (Widget && FSlateApplication::Get().AnyMenusVisible())
    {
        FWidgetPath Path;
        FSlateApplication::Get().GeneratePathToWidgetUnchecked(Widget.ToSharedRef(), Path);
        if (Path.IsValid() && FSlateApplication::Get().FindMenuInWidgetPath(Path))
        {
            for (auto Control = Widget; Control; Control = Control->GetParentWidget())
            {
                if (Control->SupportsKeyboardFocus())
                {
                    NativePopupFocus = Control;
                    return true;
                }
            }
        }
    }
    return false;
}

bool UNsUIUMGSelectionMode::GetCursorPosition(FVector2D& Position) const
{
    UWidget* Target = GetFocusTarget();
    if (!Target || !Target->GetCachedWidget())
    {
        return false;
    }
    TSharedPtr<SWidget> Native = NativePopupFocus.Pin();
    FWidgetPath Path;
    if (Native) { FSlateApplication::Get().GeneratePathToWidgetUnchecked(Native.ToSharedRef(), Path); }
    const FGeometry& Geometry = Native && Path.IsValid() ? Native->GetCachedGeometry() : Target->GetCachedGeometry();
    if (Geometry.GetLocalSize().IsNearlyZero())
    {
        return false;
    }
    FVector2D Pixels;
    USlateBlueprintLibrary::AbsoluteToViewport(Target, Geometry.LocalToAbsolute(Geometry.GetLocalSize() * FVector2D(0.f, .5f)), Pixels, Position);
    return true;
}

FString UNsUIUMGSelectionMode::DescribeSelection() const
{
    return GetNameSafe(Selected.Get());
}

bool UNsUIWorldSelectionMode::Project(int32 Index, FVector2D& Position) const
{
    FVector WorldPosition;
    APlayerController* Player = GetPlayerController();
    if (!Player || !GetCandidatePosition(Index, WorldPosition)
        || !UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(Player, WorldPosition, Position, false))
    {
        return false;
    }
    const FVector2D Size = UWidgetLayoutLibrary::GetViewportSize(Screen.Get())
        / FMath::Max(UWidgetLayoutLibrary::GetViewportScale(Screen.Get()), UE_SMALL_NUMBER);
    return Position.X >= 0 && Position.Y >= 0 && Position.X <= Size.X && Position.Y <= Size.Y;
}

bool UNsUIWorldSelectionMode::HasSelection() const
{
    FVector2D Position;
    return SelectedIndex != INDEX_NONE && Project(SelectedIndex, Position);
}

bool UNsUIWorldSelectionMode::Restore(UWidget*)
{
    if (HasSelection())
    {
        return true;
    }
    SelectedIndex = INDEX_NONE;
    FVector2D Position;
    for (int32 Index = 0; Index < GetCandidateCount(); ++Index)
    {
        if (Project(Index, Position))
        {
            SelectedIndex = Index;
            break;
        }
    }
    return HasSelection();
}

bool UNsUIWorldSelectionMode::Navigate(EUINavigation Direction)
{
    Restore(nullptr);
    FVector2D Origin;
    if (!Project(SelectedIndex, Origin))
    {
        return true;
    }
    FVector2D Axis;
    switch (Direction)
    {
    case EUINavigation::Up: Axis = FVector2D(0, -1); break;
    case EUINavigation::Down: Axis = FVector2D(0, 1); break;
    case EUINavigation::Left: Axis = FVector2D(-1, 0); break;
    case EUINavigation::Right: Axis = FVector2D(1, 0); break;
    case EUINavigation::Next: case EUINavigation::Previous:
        for (int32 Offset = 1; Offset < GetCandidateCount(); ++Offset)
        {
            const int32 Index = (SelectedIndex + (Direction == EUINavigation::Next ? Offset : GetCandidateCount() - Offset)) % GetCandidateCount();
            FVector2D Position;
            if (Project(Index, Position))
            {
                SelectedIndex = Index;
                break;
            }
        }
        return true;
    default: return false;
    }
    double BestScore = TNumericLimits<double>::Max();
    int32 Best = SelectedIndex;
    for (int32 Index = 0; Index < GetCandidateCount(); ++Index)
    {
        FVector2D Position;
        if (Index == SelectedIndex || !Project(Index, Position))
        {
            continue;
        }
        const FVector2D Delta = Position - Origin;
        const double Forward = FVector2D::DotProduct(Delta, Axis);
        if (Forward <= 0)
        {
            continue;
        }
        const double Side = FMath::Abs(Delta.X * Axis.Y - Delta.Y * Axis.X);
        const double Score = Delta.Size() + Side * 2.;
        if (Score < BestScore)
        {
            BestScore = Score;
            Best = Index;
        }
    }
    SelectedIndex = Best;
    return true;
}

bool UNsUIWorldSelectionMode::GetCursorPosition(FVector2D& Position) const
{
    return Project(SelectedIndex, Position);
}

bool UNsUIWorldSelectionMode::ObservePointer(const FVector2D& Position)
{
    double Best = 24. * 24.;
    int32 Found = INDEX_NONE;
    for (int32 Index = 0; Index < GetCandidateCount(); ++Index)
    {
        FVector2D Projected;
        if (Project(Index, Projected))
        {
            const double Distance = FVector2D::DistSquared(Position, Projected);
            if (Distance < Best) { Best = Distance; Found = Index; }
        }
    }
    if (Found == INDEX_NONE) { return false; }
    SelectedIndex = Found;
    return true;
}

void UNsUIActorSelectionMode::SetCandidates(const TArray<TWeakObjectPtr<AActor>>& InCandidates)
{
    const TWeakObjectPtr<AActor> Previous = GetSelectedActor();
    Candidates = InCandidates;
    SelectedIndex = Previous.IsValid() ? Candidates.IndexOfByKey(Previous) : INDEX_NONE;
    Restore(nullptr);
}

AActor* UNsUIActorSelectionMode::GetSelectedActor() const
{
    return Candidates.IsValidIndex(SelectedIndex) ? Candidates[SelectedIndex].Get() : nullptr;
}

int32 UNsUIActorSelectionMode::GetCandidateCount() const { return Candidates.Num(); }

bool UNsUIActorSelectionMode::GetCandidatePosition(int32 Index, FVector& Position) const
{
    if (!Candidates.IsValidIndex(Index) || !Candidates[Index].IsValid() || Candidates[Index]->IsActorBeingDestroyed())
    {
        return false;
    }
    Position = Candidates[Index]->GetActorLocation();
    return true;
}

FString UNsUIActorSelectionMode::DescribeSelection() const { return GetNameSafe(GetSelectedActor()); }

void UNsUILocationSelectionMode::SetCandidates(const TArray<FNsUILocation>& InCandidates)
{
    const FName Previous = GetSelectedId();
    Candidates.Reset();
    for (const FNsUILocation& Candidate : InCandidates)
    {
        if (!Candidate.Id.IsNone() && !Candidates.ContainsByPredicate([&](const FNsUILocation& Item) { return Item.Id == Candidate.Id; }))
        {
            Candidates.Add(Candidate);
        }
    }
    SelectedIndex = Candidates.IndexOfByPredicate([&](const FNsUILocation& Item) { return Item.Id == Previous; });
    Restore(nullptr);
}

FName UNsUILocationSelectionMode::GetSelectedId() const
{
    return Candidates.IsValidIndex(SelectedIndex) ? Candidates[SelectedIndex].Id : NAME_None;
}

int32 UNsUILocationSelectionMode::GetCandidateCount() const { return Candidates.Num(); }

bool UNsUILocationSelectionMode::GetCandidatePosition(int32 Index, FVector& Position) const
{
    if (!Candidates.IsValidIndex(Index))
    {
        return false;
    }
    Position = Candidates[Index].Position;
    return true;
}

FString UNsUILocationSelectionMode::DescribeSelection() const { return GetSelectedId().ToString(); }
