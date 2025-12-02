// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUISubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "NsUICursor.h"
#include "NsUILayout.h"
#include "NsUILayer.h"
#include "NsUISettings.h"
#include "Stats/Stats.h"

UNsUISubsystem::UNsUISubsystem()
    : ActiveUILayout(nullptr)
    , RetroCursor(nullptr)
    , RetroCursorClass(nullptr)
    , ActorSelectionIndex(INDEX_NONE)
    , bCursorTracksActorSelection(true)
{
}

void UNsUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (const UNsUISettings* Settings = GetDefault<UNsUISettings>())
    {
        RetroCursorClass = Settings->CursorWidgetClass;
    }
}

void UNsUISubsystem::Deinitialize()
{
    DestroyActiveLayout();
    RetroCursor = nullptr;
    ActorSelectionCandidates.Empty();
    ActorSelectionIndex = INDEX_NONE;

    Super::Deinitialize();
}

void UNsUISubsystem::Tick(float DeltaTime)
{
    UpdateRetroCursorFromActorSelection();
}

TStatId UNsUISubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UNsUISubsystem, STATGROUP_Tickables);
}

UNsUISubsystem* UNsUISubsystem::GetSubsystem()
{
    if (const UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UNsUISubsystem>();
        }
    }
    return nullptr;
}

UUserWidget* UNsUISubsystem::PushWidgetToLayer(const TSoftClassPtr<UUserWidget>& InWidget, ENsUILayerType InLayerType, bool bCollapsePrevious)
{
    if (!ActiveUILayout)
    {
        return nullptr;
    }

    if (UNsUIWidgetStackElement* StackElement = ActiveUILayout->PushContentToLayer(InLayerType, InWidget, bCollapsePrevious))
    {
        return StackElement->Widget.Get();
    }
    return nullptr;
}

UUserWidget* UNsUISubsystem::ReplaceTopWidgetOnLayer(const TSoftClassPtr<UUserWidget>& InWidget, ENsUILayerType InLayerType, bool bCollapsePrevious)
{
    if (!ActiveUILayout)
    {
        return nullptr;
    }

    if (UNsUIWidgetStackElement* StackElement = ActiveUILayout->ReplaceTopWidgetOnLayer(InLayerType, InWidget, bCollapsePrevious))
    {
        return StackElement->Widget.Get();
    }
    return nullptr;
}

void UNsUISubsystem::PopWidgetFromLayer(ENsUILayerType InLayerType)
{
    if (ActiveUILayout)
    {
        ActiveUILayout->PopContentFromLayer(InLayerType);
    }
}

void UNsUISubsystem::UnwindLayer(ENsUILayerType InLayerType)
{
    if (ActiveUILayout)
    {
        ActiveUILayout->UnwindLayer(InLayerType);
    }
}

void UNsUISubsystem::RemoveWidgetFromLayer(ENsUILayerType InLayerType, UUserWidget* InWidget)
{
    if (ActiveUILayout)
    {
        ActiveUILayout->RemoveWidgetFromLayer(InLayerType, InWidget);
    }
}

void UNsUISubsystem::ClearLayer(ENsUILayerType InLayerType)
{
    if (ActiveUILayout)
    {
        ActiveUILayout->ClearLayer(InLayerType);
    }
}

UNsUIWidgetStackElement* UNsUISubsystem::ApplyLayerTransition(const FNsUILayerTransitionRequest& Request)
{
    switch (Request.Operation)
    {
    case ENsUILayerTransitionOp::Push:
        return ActiveUILayout ? ActiveUILayout->PushContentToLayer(Request.LayerType, Request.WidgetClass, Request.bCollapsePrevious) : nullptr;
    case ENsUILayerTransitionOp::ReplaceTop:
        return ActiveUILayout ? ActiveUILayout->ReplaceTopWidgetOnLayer(Request.LayerType, Request.WidgetClass, Request.bCollapsePrevious) : nullptr;
    case ENsUILayerTransitionOp::Pop:
        return ActiveUILayout ? ActiveUILayout->PopContentFromLayer(Request.LayerType) : nullptr;
    case ENsUILayerTransitionOp::Remove:
        return ActiveUILayout ? ActiveUILayout->RemoveWidgetFromLayer(Request.LayerType, Request.WidgetInstance) : nullptr;
    case ENsUILayerTransitionOp::ClearLayer:
        if (ActiveUILayout)
        {
            ActiveUILayout->ClearLayer(Request.LayerType);
        }
        return nullptr;
    case ENsUILayerTransitionOp::Unwind:
        return ActiveUILayout ? ActiveUILayout->UnwindLayer(Request.LayerType).Last(nullptr) : nullptr;
    default:
        return nullptr;
    }
}

void UNsUISubsystem::DestroyActiveLayout()
{
    if (ActiveUILayout)
    {
        ActiveUILayout->RemoveFromParent();
        ActiveUILayout = nullptr;
    }
}

void UNsUISubsystem::SetActiveUILayout(const TSoftClassPtr<UNsUILayout>& InLayout)
{
    if (!GetWorld())
    {
        return;
    }

    UClass* LayoutClass = InLayout.LoadSynchronous();
    if (!LayoutClass)
    {
        DestroyActiveLayout();
        return;
    }

    if (ActiveUILayout && ActiveUILayout->GetClass() == LayoutClass)
    {
        return;
    }

    DestroyActiveLayout();

    ActiveUILayout = CreateWidget<UNsUILayout>(GetWorld(), LayoutClass);
    if (ActiveUILayout)
    {
        ActiveUILayout->AddToViewport();
    }
}

UNsUILayout* UNsUISubsystem::GetActiveUILayout() const
{
    return ActiveUILayout;
}

void UNsUISubsystem::SetRetroCursorClass(const TSoftClassPtr<UNsUICursor>& InCursorClass)
{
    RetroCursorClass = InCursorClass;
    if (RetroCursor)
    {
        RetroCursor->RemoveFromParent();
        RetroCursor = nullptr;
    }
}

UNsUICursor* UNsUISubsystem::EnsureRetroCursor()
{
    if (!RetroCursor)
    {
        UClass* CursorClass = RetroCursorClass.IsNull() ? UNsUICursor::StaticClass() : RetroCursorClass.LoadSynchronous();
        if (!CursorClass)
        {
            return nullptr;
        }

        if (UWorld* World = GetWorld())
        {
            RetroCursor = CreateWidget<UNsUICursor>(World, CursorClass);
            if (RetroCursor)
            {
                RetroCursor->AddToViewport(10000);
            }
        }
    }
    return RetroCursor;
}

void UNsUISubsystem::MoveRetroCursorToWidget(UWidget* TargetWidget)
{
    if (UNsUICursor* Cursor = EnsureRetroCursor())
    {
        Cursor->MoveToWidget(TargetWidget);
    }
}

void UNsUISubsystem::MoveRetroCursorToScreenPosition(const FVector2D& ScreenPosition)
{
    if (UNsUICursor* Cursor = EnsureRetroCursor())
    {
        Cursor->MoveToScreenPosition(ScreenPosition);
    }
}

void UNsUISubsystem::StartActorSelectionSession(const TArray<TSoftObjectPtr<AActor>>& Actors, bool bSnapCursorToSelection)
{
    ActorSelectionCandidates.Empty();
    ActorSelectionIndex = INDEX_NONE;

    for (const TSoftObjectPtr<AActor>& ActorPtr : Actors)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            ActorSelectionCandidates.Add(Actor);
        }
        else if (AActor* LoadedActor = ActorPtr.LoadSynchronous())
        {
            ActorSelectionCandidates.Add(LoadedActor);
        }
    }

    if (ActorSelectionCandidates.Num() > 0)
    {
        ActorSelectionIndex = 0;
        if (bSnapCursorToSelection)
        {
            UpdateRetroCursorFromActorSelection();
        }
    }
}

void UNsUISubsystem::ExitActorSelectionSession()
{
    ActorSelectionCandidates.Empty();
    ActorSelectionIndex = INDEX_NONE;
}

AActor* UNsUISubsystem::GetCurrentActorSelection() const
{
    return ActorSelectionCandidates.IsValidIndex(ActorSelectionIndex)
        ? ActorSelectionCandidates[ActorSelectionIndex].Get()
        : nullptr;
}

AActor* UNsUISubsystem::StepActorSelection(int32 Direction, bool bSnapCursorToSelection)
{
    if (ActorSelectionCandidates.Num() == 0)
    {
        return nullptr;
    }

    if (!ActorSelectionCandidates.IsValidIndex(ActorSelectionIndex))
    {
        ActorSelectionIndex = 0;
    }
    else
    {
        ActorSelectionIndex = (ActorSelectionIndex + Direction) % ActorSelectionCandidates.Num();
        if (ActorSelectionIndex < 0)
        {
            ActorSelectionIndex += ActorSelectionCandidates.Num();
        }
    }

    if (bSnapCursorToSelection)
    {
        UpdateRetroCursorFromActorSelection();
    }

    return GetCurrentActorSelection();
}

bool UNsUISubsystem::ProjectActorToScreen(const AActor* Actor, FVector2D& OutScreenPos) const
{
    OutScreenPos = FVector2D::ZeroVector;

    if (!Actor)
    {
        return false;
    }

    if (const UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            return PC->ProjectWorldLocationToScreen(Actor->GetActorLocation(), OutScreenPos);
        }
    }

    return false;
}

void UNsUISubsystem::UpdateRetroCursorFromActorSelection()
{
    if (!bCursorTracksActorSelection)
    {
        return;
    }

    if (UNsUICursor* Cursor = EnsureRetroCursor())
    {
        if (AActor* Actor = GetCurrentActorSelection())
        {
            FVector2D ScreenPos;
            if (ProjectActorToScreen(Actor, ScreenPos))
            {
                Cursor->MoveToScreenPosition(ScreenPos);
                return;
            }
        }

        Cursor->SetVisibility(ESlateVisibility::Collapsed);
    }
}
