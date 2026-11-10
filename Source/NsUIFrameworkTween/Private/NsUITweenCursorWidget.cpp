#include "NsUITweenCursorWidget.h"

#if WITH_NSTWEEN
#include "NsTween.h"
#endif

/** The NsTween builder of one glide. Kept out of the header, so the header compiles without NsTween */
struct FNsUITweenCursorGlide
{
#if WITH_NSTWEEN
    /** The tween of the glide */
    FNsTweenBuilder Tween;
#endif
};

UNsUITweenCursorWidget::UNsUITweenCursorWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    GlideSeconds = 0.12f;
    ShownPosition = FVector2D::ZeroVector;
}

void UNsUITweenCursorWidget::NativeDestruct()
{
#if WITH_NSTWEEN
    if (Glide.IsValid())
    {
        Glide->Tween.Cancel(false);
    }
#endif
    Glide.Reset();
    Super::NativeDestruct();
}

void UNsUITweenCursorWidget::MoveCursor(const FVector2D& ViewportPosition, const bool bInstant)
{
#if WITH_NSTWEEN
    if (Glide.IsValid())
    {
        Glide->Tween.Cancel(false);
    }
    if (!bInstant && GlideSeconds > 0.f)
    {
        if (!Glide.IsValid())
        {
            Glide = MakePimpl<FNsUITweenCursorGlide>();
        }
        TWeakObjectPtr<UNsUITweenCursorWidget> WeakThis(this);
        Glide->Tween = FNsTween::Play(ShownPosition, ViewportPosition, GlideSeconds, ENsTweenEase::OutCubic, [WeakThis](const FVector2D& Value)
        {
            if (UNsUITweenCursorWidget* const Cursor = WeakThis.Get())
            {
                Cursor->ShownPosition = Value;
                Cursor->UNsUICursorWidget::MoveCursor(Value, true);
            }
        });
        Glide->Tween.SetOwner(this);
        return;
    }
#endif
    ShownPosition = ViewportPosition;
    Super::MoveCursor(ViewportPosition, bInstant);
}
