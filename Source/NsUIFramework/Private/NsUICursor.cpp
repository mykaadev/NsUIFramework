#include "NsUICursor.h"

#include "SlateBasics.h"
#include "Styling/AppStyle.h"
#include "Widgets/Images/SImage.h"

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
