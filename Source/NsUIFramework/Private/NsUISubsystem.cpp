// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsUISubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "NsUILayout.h"
#include "NsUILayer.h"
#include "NsUICursor.h"

UNsUISubsystem::UNsUISubsystem()
    : ActiveUILayout(nullptr)
{
}

void UNsUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UNsUISubsystem::Deinitialize()
{
    ActiveUILayout = nullptr;
    Super::Deinitialize();
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

UUserWidget* UNsUISubsystem::ReplaceTopWidgetOnLayer(const TSoftClassPtr<UUserWidget>& InWidget, ENsUILayerType InLayerType)
{
    if (!ActiveUILayout)
    {
        return nullptr;
    }
    if (UNsUIWidgetStackElement* StackElement = ActiveUILayout->ReplaceTopWidgetOnLayer(InLayerType, InWidget))
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
        return ActiveUILayout ? ActiveUILayout->ReplaceTopWidgetOnLayer(Request.LayerType, Request.WidgetClass) : nullptr;
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

void UNsUISubsystem::SetActiveUILayout(const TSoftClassPtr<UNsUILayout>& InLayout)
{
    if (!GetWorld())
    {
        return;
    }

    UClass* LayoutClass = InLayout.LoadSynchronous();
    if (!LayoutClass)
    {
        return;
    }

    if (ActiveUILayout)
    {
        ActiveUILayout->RemoveFromParent();
        ActiveUILayout = nullptr;
    }

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

void UNsUISubsystem::StartActorSelectionSession(const TArray<TSoftObjectPtr<AActor>>& Actors)
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

AActor* UNsUISubsystem::StepActorSelection(int32 Direction)
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

    return GetCurrentActorSelection();
}
