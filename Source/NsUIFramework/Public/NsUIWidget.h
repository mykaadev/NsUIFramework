#pragma once
#include "Blueprint/UserWidget.h"
#include "NsUIScreen.h"
#include "NsUIWidget.generated.h"

/** Screen parent with the same optional contract as ordinary user widgets. */
UCLASS()
class NSUIFRAMEWORK_API UNsUIWidget : public UUserWidget, public INsUIScreen
{
    GENERATED_BODY()
};
