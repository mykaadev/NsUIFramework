#include "NsUICursor.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SlateBasics.h"
#include "Styling/AppStyle.h"
#include "Widgets/Images/SImage.h"
#include "NsUILayout.h"

UNsUICursor::UNsUICursor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetVisibility(ESlateVisibility::HitTestInvisible);
    CursorBrush = *FAppStyle::Get().GetBrush("Icons.ChevronRight");
}

TSharedRef<SWidget> UNsUICursor::RebuildWidget()
{
    MyImage = SNew(SImage)
        .Image(&CursorBrush);

    return MyImage.ToSharedRef();
}

void UNsUICursor::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    if (MyImage.IsValid())
    {
        MyImage->SetImage(&CursorBrush);
    }
}

void UNsUICursor::ReleaseSlateResources(bool bWholeView)
{
    Super::ReleaseSlateResources(bWholeView);

    MyImage.Reset();
}

void UNsUICursor::SetCursorBrush(const FSlateBrush& InBrush)
{
    CursorBrush = InBrush;
    SynchronizeProperties();
}

void UNsUICursor::SetOwningLayout(UNsUILayout* InLayout)
{
    OwningLayout = InLayout;
}

void UNsUICursor::SetTargetWidget(UWidget* Widget)
{
    CursorTargetWidget = Widget;
    CursorTargetType = Widget ? ENsUICursorTargetType::Widget : ENsUICursorTargetType::None;
}

void UNsUICursor::SetTargetActor(AActor* Actor)
{
    CursorTargetActor = Actor;
    CursorTargetType = Actor ? ENsUICursorTargetType::Actor : ENsUICursorTargetType::None;
}

void UNsUICursor::SetTargetLocation(const FVector& WorldLocation)
{
    CursorTargetWorldLocation = WorldLocation;
    CursorTargetType = ENsUICursorTargetType::Location;
}

FVector2D UNsUICursor::ResolveWidgetScreenPosition(UWidget* Widget) const
{
    if (Widget == nullptr || OwningLayout == nullptr || Widget->GetCachedWidget().IsValid() == false)
    {
        return FVector2D::ZeroVector;
    }

    const FGeometry& Geometry = Widget->GetCachedGeometry();
    const FVector2D Absolute = Geometry.GetAbsolutePosition() + (Geometry.GetAbsoluteSize() * OwningLayout->GetCursorWidgetSnapAnchor());

    FVector2D PixelPosition;
    FVector2D InOutViewportPosition;
    USlateBlueprintLibrary::AbsoluteToViewport(GetWorld(), Absolute, PixelPosition, InOutViewportPosition);
    return InOutViewportPosition;
}

FVector2D UNsUICursor::ResolveActorScreenPosition(AActor* Actor) const
{
    if (!Actor || !OwningLayout)
    {
        return FVector2D::ZeroVector;
    }

    ULocalPlayer* LP = OwningLayout->GetOwningLocalPlayer();
    APlayerController* PC = LP ? LP->GetPlayerController(LP->GetWorld()) : nullptr;
    if (!PC)
    {
        return FVector2D::ZeroVector;
    }

    const FBox Bounds = Actor->GetComponentsBoundingBox(true);
    const FVector WorldCenter = Bounds.IsValid ? Bounds.GetCenter() : Actor->GetActorLocation();

    FVector2D Screen;
    if (UGameplayStatics::ProjectWorldToScreen(PC, WorldCenter, Screen))
    {
        return Screen;
    }

    return FVector2D::ZeroVector;
}

FVector2D UNsUICursor::ResolveLocationScreenPosition(const FVector& WorldLocation) const
{
    if (!OwningLayout)
    {
        return FVector2D::ZeroVector;
    }

    ULocalPlayer* LP = OwningLayout->GetOwningLocalPlayer();
    APlayerController* PC = LP ? LP->GetPlayerController(LP->GetWorld()) : nullptr;
    if (!PC)
    {
        return FVector2D::ZeroVector;
    }

    FVector2D Screen;
    if (UGameplayStatics::ProjectWorldToScreen(PC, WorldLocation, Screen))
    {
        return Screen;
    }

    return FVector2D::ZeroVector;
}

FVector2D UNsUICursor::ResolveDesiredScreenPosition() const
{
    switch (CursorTargetType)
    {
    case ENsUICursorTargetType::Widget:
        if (CursorTargetWidget.IsValid())
        {
            return ResolveWidgetScreenPosition(CursorTargetWidget.Get());
        }
        break;
    case ENsUICursorTargetType::Actor:
        if (CursorTargetActor.IsValid())
        {
            return ResolveActorScreenPosition(CursorTargetActor.Get());
        }
        break;
    case ENsUICursorTargetType::Location:
        return ResolveLocationScreenPosition(CursorTargetWorldLocation);
    default:
        break;
    }

    return FVector2D::ZeroVector;
}

void UNsUICursor::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!OwningLayout || CursorTargetType == ENsUICursorTargetType::None)
    {
        return;
    }

    if (CursorTargetType == ENsUICursorTargetType::Widget && (!CursorTargetWidget.IsValid() || !CursorTargetWidget->GetCachedWidget().IsValid()))
    {
        CursorTargetType = ENsUICursorTargetType::None;
        CursorTargetWidget.Reset();
        return;
    }

    if (CursorTargetType == ENsUICursorTargetType::Actor && !CursorTargetActor.IsValid())
    {
        CursorTargetType = ENsUICursorTargetType::None;
        CursorTargetActor.Reset();
        return;
    }

    const FVector2D DesiredPosition = ResolveDesiredScreenPosition();
    if (!DesiredPosition.IsNearlyZero() || CursorTargetType == ENsUICursorTargetType::Location)
    {
        CursorTargetPosition = DesiredPosition;
    }

    if (CursorTargetPosition.IsNearlyZero())
    {
        return;
    }

    if (!bCursorHasPosition)
    {
        CurrentCursorPosition = CursorTargetPosition;
        bCursorHasPosition = true;
    }
    else
    {
        const float Distance = FVector2D::Distance(CurrentCursorPosition, CursorTargetPosition);
        if (Distance > 1.f)
        {
            const float Alpha = FMath::Clamp(InDeltaTime * CursorLerpSpeed, 0.f, 1.f);
            const float EasedAlpha = FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.f);
            CurrentCursorPosition = FMath::Lerp(CurrentCursorPosition, CursorTargetPosition, EasedAlpha);
        }
        else
        {
            CurrentCursorPosition = CursorTargetPosition;
        }
    }

    const FVector2D Size = GetDesiredSize();
    const FVector2D Offset = Size * OwningLayout->GetCursorAnchor();
    SetRenderTranslation(CurrentCursorPosition - Offset);
}
