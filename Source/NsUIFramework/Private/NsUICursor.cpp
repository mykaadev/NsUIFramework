#include "NsUICursor.h"
#include "NsUIDeveloperSettings.h"
#include "Styling/AppStyle.h"
#include "Widgets/Images/SImage.h"

UNsUICursor::UNsUICursor(const FObjectInitializer& Initializer) : Super(Initializer)
{
    CursorBrush = *FAppStyle::Get().GetBrush("Icons.ChevronRight");
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

TSharedRef<SWidget> UNsUICursor::RebuildWidget()
{
    return SAssignNew(Image, SImage).Image(&CursorBrush);
}

void UNsUICursor::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);
    Image.Reset();
}

void UNsUICursor::SetCursorBrush(const FSlateBrush& Brush)
{
    CursorBrush = Brush;
    if (Image) { Image->SetImage(&CursorBrush); }
}

void UNsUICursor::Present(const TOptional<FVector2D>& Position, float DeltaTime)
{
    SetVisibility(Position.IsSet() ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    if (!Position.IsSet())
    {
        bHasPosition = false;
        return;
    }
    const UNsUIDeveloperSettings* Settings = GetDefault<UNsUIDeveloperSettings>();
    const float Alpha = FMath::Clamp(DeltaTime * Settings->CursorVelocity, 0.f, 1.f);
    const float Eased = FMath::Clamp(Settings->CursorLerpCurve.GetRichCurveConst()->Eval(Alpha), 0.f, 1.f);
    CurrentPosition = bHasPosition ? FMath::Lerp(CurrentPosition, Position.GetValue(), Eased) : Position.GetValue();
    bHasPosition = true;
    SetPositionInViewport(CurrentPosition, false);
}
