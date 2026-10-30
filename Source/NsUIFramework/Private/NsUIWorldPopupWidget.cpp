#include "NsUIWorldPopupWidget.h"

#include "Components/TextBlock.h"

void UNsUIWorldPopupWidget::StartPopup(const FText& Text, const FLinearColor& Color)
{
    if (PopupText)
    {
        PopupText->SetText(Text);
        PopupText->SetColorAndOpacity(FSlateColor(Color));
    }
    ApplyAge(0.f);
    OnPopupStarted(Text, Color);
}

bool UNsUIWorldPopupWidget::ApplyAge(const float Age)
{
    if (Age >= Lifetime)
    {
        return false;
    }

    const float FadeIn = FadeInSeconds > 0.f ? FMath::Clamp(Age / FadeInSeconds, 0.f, 1.f) : 1.f;
    const float FadeOut = FadeOutSeconds > 0.f ? FMath::Clamp((Lifetime - Age) / FadeOutSeconds, 0.f, 1.f) : 1.f;
    SetRenderOpacity(FMath::Min(FadeIn, FadeOut));

    // The pop eases out, so the popup lands softly at scale 1
    const float PopAlpha = PopSeconds > 0.f ? FMath::Clamp(Age / PopSeconds, 0.f, 1.f) : 1.f;
    const float Scale = FMath::Lerp(PopScale, 1.f, 1.f - FMath::Square(1.f - PopAlpha));
    SetRenderScale(FVector2D(Scale));
    return true;
}
